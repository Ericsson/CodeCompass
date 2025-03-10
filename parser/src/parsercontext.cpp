#include <fstream>

#include <boost/filesystem.hpp>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <util/hash.h>
#include <util/odbtransaction.h>

#include <parser/parsercontext.h>
#include <parser/sourcemanager.h>

namespace po = boost::program_options;
namespace fs = boost::filesystem;

namespace cc
{
namespace parser
{

ParserContext::ParserContext(
  std::shared_ptr<odb::database> db_,
  SourceManager& srcMgr_,
  std::string& compassRoot_,
  po::variables_map& options_) :
    db(db_),
    srcMgr(srcMgr_),
    compassRoot(compassRoot_),
    options(options_)
{
  std::unordered_map<std::string, std::string> fileHashes;

  (util::OdbTransaction(this->db))([&]
   {
     // Fetch directory and binary type files from SourceManager
     auto func = [](model::FilePtr item)
     {
       return item->type != model::File::DIRECTORY_TYPE &&
              item->type != model::File::BINARY_TYPE;
     };
     std::vector<model::FilePtr> files = this->srcMgr.getFiles(func);

     for (model::FilePtr file : files)
     {
       if (boost::filesystem::exists(file->path))
       {
         if (!fileStatus.count(file->path))
         {
           model::FileContentPtr content = file->content.load();
           if (!content)
             continue;

           fileHashes[file->path] = content->hash;

           std::ifstream fileStream(file->path);
           std::string fileContent(
             std::istreambuf_iterator<char>{fileStream},
             std::istreambuf_iterator<char>{});
           fileStream.close();

           if (content->hash != util::sha1Hash(fileContent))
           {
             this->fileStatus.emplace(
               file->path, cc::parser::IncrementalStatus::MODIFIED);
             LOG(debug) << "File modified: " << file->path;
           }
         }
       }
       else
       {
         fileStatus.emplace(
           file->path, cc::parser::IncrementalStatus::DELETED);
         LOG(debug) << "File deleted: " << file->path;
       }
     }

     // TODO: detect ADDED files
   });

  // Fill moduleDirectories vector
  if (options.count("modules")) {
    const std::string& modulesFilePath = options["modules"].as<std::string>();

    std::ifstream fileStream(modulesFilePath);

    if (!fileStream.good()) {
      LOG(error) << "Failed to open modules file: " << modulesFilePath;
      exit(1);
    }

    LOG(info) << "Processing modules file: " << modulesFilePath;

    std::string line;
    while (std::getline(fileStream, line)) {
      try {
        const fs::path p = fs::canonical(line);
        moduleDirectories.push_back(p.string());
      } catch (...) {
        LOG(error) << "Failed to process path from modules file: " << line;
        exit(1);
      }
    }
  }
}
}
}

