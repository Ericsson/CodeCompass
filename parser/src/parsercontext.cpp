#include <parser/parsercontext.h>
#include <parser/sourcemanager.h>

namespace po = boost::program_options;

namespace cc
{,
namespace parser
{

ParserContext::ParserContext(
    std::shared_ptr<odb::database> db_,
    SourceManager& srcMgr_,
    std::string& compassRoot_,
    po::variables_map& options_,
    std::unordered_map<std::string, IncrementalStatus> fileStatus_):
    db(db_),
    srcMgr(srcMgr_),
    compassRoot(compassRoot_),
    options(options_),
    fileStatus(fileStatus_)
{
  std::unordered_map<std::string, std::string> fileHashes;

  (util::OdbTransaction(this->db))([&]
   {
     // Fetch all files from SourceManager
     std::vector<model::FilePtr> files = this->srcMgr.getAllFiles();
     files.erase(std::remove_if(files.begin(), files.end(),
                                [](const auto &item)
                                {
                                  return item->type == model::File::DIRECTORY_TYPE ||
                                         item->type == model::File::BINARY_TYPE;
                                }), files.end());

     for (model::FilePtr file : files)
     {
       if (boost::filesystem::exists(file->path))
       {
         if (!this->fileStatus.count(file->path))
         {
           auto content = file->content.load();
           fileHashes[file->path] = content != nullptr ? content->hash : "";
           if (content == nullptr)
             continue;

           std::ifstream fileStream(file->path);
           std::string fileContent(
               (std::istreambuf_iterator<char>(fileStream)),
               (std::istreambuf_iterator<char>()));
           fileStream.close();

           if (content->hash != util::sha1Hash(fileContent))
           {
             this->fileStatus.insert(
               std::make_pair(file->path, cc::parser::IncrementalStatus::MODIFIED));
             LOG(debug) << "File modified: " << file->path;
           }
         }
       }
       else
       {
         this->fileStatus.insert(
             std::make_pair(file->path, cc::parser::IncrementalStatus::DELETED));
         LOG(debug) << "File deleted: " << file->path;
       }
     }

     // TODO: detect ADDED files
   });
}

}
}

