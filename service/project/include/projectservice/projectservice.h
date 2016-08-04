#ifndef CC_SERVICE_CORE_PROJECTSERVICE_H
#define CC_SERVICE_CORE_PROJECTSERVICE_H

#include <memory>

#include <odb/database.hxx>

#include <util/db/odbtransaction.h>
#include <model/file.h>

#include <ProjectService.h>

namespace cc 
{
namespace service
{
namespace core
{

class ProjectServiceHandler : virtual public ProjectServiceIf {
 public:
  ProjectServiceHandler(std::shared_ptr<odb::database> db_);

  virtual void getFileInfo(FileInfo& _return, const  FileId& file) override;
  
  virtual void getFileInfoByPath(FileInfo& _return, const  std::string& path) override;

  virtual void getFileStat(FileStat& _return, const FileId& file) override;
  
  virtual void getProjectInfo(ProjectInfo& _return, const ProjectId& project) override;
  
  virtual void getRootFiles(std::vector<FileInfo> & _return, const ProjectId& project) override;

  virtual void getChildFiles(std::vector<FileInfo> & _return, const FileId& file) override;

  virtual void getSubtree(std::vector<FileInfo> & _return, const FileId& file) override;

  virtual void getOpenTreeTillFile(std::vector<FileInfo> & _return, const FileId& file) override;

  virtual void getPathTillFile(std::vector<FileInfo> & _return, const  ::cc::service::core::FileId& file) override;
  
  virtual void getBuildLog(std::vector<BuildLog> & _return, const  ::cc::service::core::FileId& file) override;
  
  virtual void getContainingFile(FileInfo& _return, const FileId& file) override;
  
  virtual void searchFile(std::vector<FileInfo> & _return, const std::string& searchText, const bool onlyFile) override;

  virtual void getStatistics(std::vector<StatisticsInfo>& _return) override;

private:
  /**
   * This function defines an ordering among FileInfo objects. The files are
   * ordered by their names, but directories are always precede files.
   */
  static bool fileInfoOrder(const FileInfo& left, const FileInfo& right);
  
  void checkDbAndThrow();
  std::string getFileNameFromPath(std::string path);
  FileInfo makeFileInfo(model::File &f);  
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction transaction;
};

} // project
} // service
} // cc

#endif // CC_SERVICE_CORE_PROJECTSERVICE_H
