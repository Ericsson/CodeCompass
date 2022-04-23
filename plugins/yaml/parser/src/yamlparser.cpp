#include <iterator>
#include <fstream>
#include <memory>

#include <boost/filesystem.hpp>

#include <util/logutil.h>
#include <util/dbutil.h>
#include <util/odbtransaction.h>
#include <util/threadpool.h>

#include <parser/sourcemanager.h>

#include <model/yaml.h>
#include <model/yaml-odb.hxx>

#include <model/yamlcontent.h>
#include <model/yamlcontent-odb.hxx>

#define RYML_SINGLE_HDR_DEFINE_NOW
#include "yamlparser/ryml_all.hpp"

#include <yamlparser/yamlparser.h>

namespace cc
{
namespace parser
{


template<class CharContainer>
size_t file_get_contents(const char *filename, CharContainer *v)
{
    ::FILE *fp = ::fopen(filename, "rb");
    C4_CHECK_MSG(fp != nullptr, "could not open file");
    ::fseek(fp, 0, SEEK_END);
    long sz = ::ftell(fp);
    v->resize(static_cast<typename CharContainer::size_type>(sz));
    if(sz)
    {
        ::rewind(fp);
        size_t ret = ::fread(&(*v)[0], 1, v->size(), fp);
        C4_CHECK(ret == (size_t)sz);
    }
    ::fclose(fp);
    return v->size();
}

/** load a file from disk into an existing CharContainer */
template<class CharContainer>
CharContainer file_get_contents(const char *filename)
{
    CharContainer cc;
    file_get_contents(filename, &cc);
    return cc;
}

/** save a buffer into a file */
template<class CharContainer>
void file_put_contents(const char *filename, CharContainer const& v, const char* access)
{
    file_put_contents(filename, v.empty() ? "" : &v[0], v.size(), access);
}

/** save a buffer into a file */
void file_put_contents(const char *filename, const char *buf, size_t sz, const char* access)
{
    ::FILE *fp = ::fopen(filename, access);
    C4_CHECK_MSG(fp != nullptr, "could not open file");
    ::fwrite(buf, 1, sz, fp);
    ::fclose(fp);
}


// std::vector<YamlParser::keyData> YamlParser::duplicate(std::vector<YamlParser::keyData> kdata)
// {
//   for (keyData kd : kdata)
//   {
//     LOG(info) <<"In DUPLICATE: " << kd.key << " " << " " << kd.data << std::endl;
//   }
//   return kdata;
// }


YamlParser::YamlParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  // util::OdbTransaction {_ctx.db} ([&, this] {
  //   for (const model::MetricsFileIdView& mf
  //     : _ctx.db->query<model::MetricsFileIdView>())
  //   {
  //     _fileIdCache.insert(mf.file);
  //   }
  // });

  ///MetricsFileIdView actually Contains Metrics db object, thats why we are only querying
  ///MetricsFileIdView, so in case of yaml, as we dont have YamlFileIdView, probably we should
  ///query Yaml

  ///QUESTION: Should I add YamlContent db object to the _fileIdCache as well? 

  util::OdbTransaction {_ctx.db} ([&, this] {
    for (const model::Yaml& yf  ///QUESTION: Tried model::Yaml here as well
      : _ctx.db->query<model::Yaml>())
    {
      _fileIdCache.insert(yf.file);
    }
  });

  int threadNum = 1;//_ctx.options["jobs"].as<int>();///Not needed probably
  _pool = util::make_thread_pool<std::string>(
    threadNum, [this](const std::string& path_)
    {
      model::FilePtr file = _ctx.srcMgr.getFile(path_);
      if (file)
      {
        if (_fileIdCache.find(file->id) == _fileIdCache.end())
        {
          if (accept(file->path)) 
          {
            // Type type;
            // std::vector<keyData> ked = getDataFromFile(file, type);

            // LOG(info) << "In Ctr, size is: " << ked.size() << std::endl;
   
            // for (keyData kd : ked)
            // {
            //   LOG(info) <<"In CTR: " << kd.key << " " << " " << kd.data << std::endl;
            // }
            // std::vector<keyData> final = duplicate(ked);
            // for (keyData kd : final)
            // {
            //   LOG(info) <<"In CTR FINAL: " << kd.key << " " << " " << kd.data << std::endl;
            // }
            // LOG(info) << "DEBUG: In Ctr path is: " << path_ << std::endl;
            this->persistData(file, file->id);
            ++this->_visitedFileCount;
          }
          
        }
        else
          LOG(debug) << "YamlParser already parsed this file: " << file->path;
      }
    });

  ///model::FilePtr file = _ctx.srcMgr.getFile(path_);
  ///I believe this would be a yaml file, somehow coming from the
  ///the project under parsing

  // if (file)
  // {
  //   if (_fileIdCache.find(file->id) == _fileIdCache.end())
  //   {
  //     this->persistLoc(keyData(), file->id);
  //     ///persistLoc will add the useful data to the db
  //     ///getLoc will get the useful data from the yaml file
  //     ++this->_visitedFileCount;
  //   }
  //   else
  //   {
  //     LOG(debug) << "Already parsed this yaml file: " << file->path;
  //   }
  // }
}

bool YamlParser::cleanupDatabase()///I guess it comes later, can be omitted
{
  if (!_fileIdCache.empty())
  {
    try
    {
      util::OdbTransaction {_ctx.db} ([this] {
        for (const model::File& file
          : _ctx.db->query<model::File>(
          odb::query<model::File>::id.in_range(_fileIdCache.begin(), _fileIdCache.end())))
        {
          auto it = _ctx.fileStatus.find(file.path);
          if (it != _ctx.fileStatus.end() &&
              (it->second == cc::parser::IncrementalStatus::DELETED ||
               it->second == cc::parser::IncrementalStatus::MODIFIED ||
               it->second == cc::parser::IncrementalStatus::ACTION_CHANGED))
          {
            LOG(info) << "[yamlparser] Database cleanup: " << file.path;

            _ctx.db->erase_query<model::Yaml>(odb::query<model::Yaml>::file == file.id);
            _fileIdCache.erase(file.id);
          }
        }
      });
    }
    catch (odb::database_exception&)
    {
      LOG(fatal) << "Transaction failed in yaml parser!";
      return false;
    }
  }
  return true;
}

bool YamlParser::parse()
{
  this->_visitedFileCount = 0;

  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    LOG(info) << "Yaml parse path: " << path;

    util::OdbTransaction trans(_ctx.db);
    trans([&, this]() {
      auto cb = getParserCallback();
      // auto cb = accept(path);
      /*--- Call non-empty iter-callback for all files
         in the current root directory. ---*/
      try
      {
        
        util::iterateDirectoryRecursive(path, cb);
      }
      catch (std::exception& ex_)
      {
        LOG(warning)
          << "Yaml parser threw an exception: " << ex_.what();
      }
      catch (...)
      {
        LOG(warning)
          << "Yaml parser failed with unknown exception!";
      }

    });
  }

  _pool->wait();
  LOG(info) << "Processed files: " << this->_visitedFileCount;

  return true;
}
///Snippet from util/include/parseutil.h

/**
 * Callback function type for iterateDirectoryRecursive.
 *
 * The parameter is the full path of the current entity.
 * If the callback returns false, then the iteration stops.
 */
///typedef std::function<bool (const std::string&)> DirIterCallback;

/**
 * Recursively iterate over the given directory.
 * @param path_ Directory or a regular file.
 * @param callback_ Callback function which will be called on each existing
 * path_. If this callback returns false then the files under the current
 * directory won't be iterated.
 * @return false if the callback_ returns false on the given path_.
 */
// bool iterateDirectoryRecursive(
//   const std::string& path_,
//   DirIterCallback callback_);
 
///So, as per my understanding, this getParserCallback should return true
///Only for yaml files and the dirs which contain yaml files
util::DirIterCallback YamlParser::getParserCallback()
{
  return [this](const std::string& currPath_)
  {
    boost::filesystem::path path(currPath_);

    if (boost::filesystem::is_regular_file(path))
    {
      // if (accept(currPath_))
        _pool->enqueue(currPath_);

    }
    
    return true;
  };
}

bool YamlParser::accept(const std::string& path_) const
{
  std::string ext = boost::filesystem::extension(path_);
  std::string cmd = "file " + path_ + " >test.txt";
  int status = std::system(cmd.c_str()); // execute the UNIX command "ls -l >test.txt"
  std::string tmp, ascii, text;
  std::ifstream("test.txt") >> tmp >> ascii >> text;
  std::system("rm -rf test.txt");
  std::string fileType = ascii + " " + text;
  return ext == ".yaml" && fileType == "ASCII text";// && result == "ASCII text";
}
/**
* Here I think we can use our yamlparsering libraries and grab the useful content
* from the files and then tranform them to put them into db
*/
std::vector<YamlParser::keyData> YamlParser::getDataFromFile(model::FilePtr file_, Type &type) const
{
  std::vector<keyData> keysDataPairs;
  if (accept(file_->path))
  {
    std::string contents =  file_get_contents<std::string>(file_->path.c_str());///file_->content ? file_->content.load()->content : std::string("nothing here");
    // LOG(info) << "DEBUG: In getDataFromFile, contents are: " << contents << std::endl;
    ryml::Tree yamlTree = ryml::parse_in_arena(ryml::to_csubstr(contents));
    if (yamlTree["apiVersion"].has_key())
    {
      if (yamlTree["name"].has_key() && yamlTree["version"].has_key())
      {
        type = Type::HELM_CHART;
      }
      else if (yamlTree["kind"].has_key())
      {
        type = Type::KUBERNETES_CONFIG;
      }
    }
    else 
    {
      type = Type::OTHER;
    }
    // std::stringstream ss;
    // ss << yamlTree;
    // LOG(info) << "DEBUG: In getDataFromFile, tree is: " << ss.str() << std::endl;
    ryml::NodeRef root = yamlTree.rootref();
    
    
    for (ryml::NodeRef n : root.children())
    {
      // LOG(info) << "DEBUG: In getDataFromFile, loopworks: " <<std::endl;
      keysDataPairs.push_back(keyData(n.has_key() ? n.key() : ryml::csubstr{}, n.has_val() ? n.val() : ryml::csubstr{}));
    }

    LOG(info) << "DEBUG: In getDataFromFile, size is: " << keysDataPairs.size() << std::endl;
    for (keyData kd : keysDataPairs)
    {
      LOG(info) <<" In getDataFromFile " << kd;
    }
  }
  return keysDataPairs;

}


/**
* Getting Loc(useful data) and putting it into the db, it is actually populating the
* db object we created in model/yaml.h. It is using transaction
*/
// void YamlParser::persistData(const std::vector<keyData>& data_, model::FileId file_, Type type)
void YamlParser::persistData(model::FilePtr file_, model::FileId fileId_)
{
  Type type;
  std::vector<keyData> keysDataPairs;
  std::string contents =  file_get_contents<std::string>(file_->path.c_str());///file_->content ? file_->content.load()->content : std::string("nothing here");
  ryml::Tree yamlTree = ryml::parse_in_arena(ryml::to_csubstr(contents));
  if (yamlTree["apiVersion"].has_key()) 
  {
    if (yamlTree["name"].has_key() && yamlTree["version"].has_key())
    {
      type = Type::HELM_CHART;
    }
    else if (yamlTree["kind"].has_key())
    {
      type = Type::KUBERNETES_CONFIG;
    }
  }
  else if (isCI(file_->path, "ci.yaml") || isCI(file_->path, "ci.yml") )
  {
    type = Type::CI;
  }
  else 
  {
    type = Type::OTHER;
  }
  ryml::NodeRef root = yamlTree.rootref();
  
  
  for (ryml::NodeRef n : root.children())
  {
    keysDataPairs.push_back(keyData(n.has_key() ? n.key() : ryml::csubstr{}, n.has_val() ? n.val() : ryml::csubstr{}));
  }
  util::OdbTransaction trans(_ctx.db);
  trans([&, this]{
    for (keyData kd : keysDataPairs)
    {
      LOG(info) << "In PersistData: YamlKeydata is: " << kd;
   
      model::YamlContent yamlContent;
      yamlContent.file = fileId_;
      std::stringstream ss;
      ss << kd.key;
      yamlContent.key = ss.str();///ryml::emitrs<std::string>(kd.key);
      ss.str("");
      ss << kd.data;
      yamlContent.data = ss.str(); ///ryml::emitrs<std::string>(kd.data);//ryml::to_csubstr(kd.data);
      LOG(info) << "YamlContent is: " << yamlContent.key << " " << yamlContent.data << std::endl;
      _ctx.db->persist(yamlContent);
    }
    model::Yaml yaml;
    yaml.file = fileId_;
    yaml.type = model::Yaml::Type(type);
    _ctx.db->persist(yaml);
  });
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Yaml Plugin");
    return description;
  }

  std::shared_ptr<YamlParser> make(ParserContext& ctx_)
  {
    return std::make_shared<YamlParser>(ctx_);
  }
}
#pragma clang diagnostic pop

}
}
