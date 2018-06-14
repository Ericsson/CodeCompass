#ifndef CC_SERVICE_CORE_PROJECTSERVICE_H
#define CC_SERVICE_CORE_PROJECTSERVICE_H

#include <memory>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>

#include <model/file.h>
#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

#include <ProjectService.h>

namespace cc 
{
namespace service
{
namespace core
{

class ProjectServiceHandler : virtual public ProjectServiceIf {
public:
  ProjectServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void getFileInfo(FileInfo& return_, const FileId& fileId_) override;
  void getFileInfoByPath(FileInfo& return_, const std::string& path) override;
  void getFileContent(std::string& return_, const FileId& fileId_) override;
  void getRootFiles(std::vector<FileInfo>& return_) override;
  void getChildFiles(std::vector<FileInfo>& return_, const FileId& fileId_) override;
  void getSubtree(std::vector<FileInfo>& return_, const FileId& fileId_) override;
  void getOpenTreeTillFile(std::vector<FileInfo>& return_, const FileId& fileId_) override;
  void getPathTillFile(std::vector<FileInfo>& return_, const FileId& fileId_) override;
  void getBuildLog(std::vector<BuildLog>& return_, const FileId& fileId_) override;
  void getParent(FileInfo& return_, const FileId& fileId_) override;
  void searchFile(std::vector<FileInfo>& return_, const std::string& text_, const bool onlyFile_) override;
  void getStatistics(std::vector<StatisticsInfo>& return_) override;
  void getFileTypes(std::vector<std::string>& return_) override;
  void getLabels(std::map<std::string, std::string>& return_) override;

private:
  /**
   * This function defines an ordering among FileInfo objects. The files are
   * ordered by their names, but directories are always precede files.
   */
  static bool fileInfoOrder(const FileInfo& left, const FileInfo& right);

  FileInfo makeFileInfo(model::File &f_);

  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  std::string _datadir;
};

} // project
} // service
} // cc

#endif // CC_SERVICE_CORE_PROJECTSERVICE_H
