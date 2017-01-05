#include <odb/transaction.hxx>
#include <odb/query.hxx>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>
#include <util/util.h>

#include <model/project.h>
#include <model/project-odb.hxx>
#include <model/option.h>
#include <model/option-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/buildlog.h>
#include <model/buildlog-odb.hxx>
#include "model/statistics.h"
#include "model/statistics-odb.hxx"

#include "projectservice.h"

namespace cc
{
namespace service
{
namespace core
{
  
  ProjectServiceHandler::ProjectServiceHandler(std::shared_ptr<odb::database> db_)
  : _db(db_), transaction(db_)
  {    
  }
  
  void ProjectServiceHandler::getFileInfo(FileInfo& _return, const FileId& file)
  {
    transaction ([&, this](){
        try
        {
          model::File f;
          
          if (!_db->find<model::File>(stoull(file.fid), f))
          {
            SLog(util::ERROR) << "invalid file ID: " << file.fid;
            
            InvalidId ex;
            ex.fid = file.fid;
            ex.what = "invalid file ID";
            throw ex;
          }
          
          _return=makeFileInfo(f);
        }
        catch (odb::exception &odbex)
        {
          SLog(util::ERROR) << "datasource exception occured: " << odbex.what();
          
          DatasourceError ex;
          ex.what = odbex.what();
          throw ex;
        }       
    });
    
  }
  
  void ProjectServiceHandler::getFileInfoByPath(FileInfo& _return, const std::string& path)
  {
    transaction([&, this]() {
      typedef odb::query<model::File> fileQuery;
      typedef odb::result<model::File> fileResult;
      
      fileResult res = _db->query<model::File>(fileQuery::path == path);
      
      if (res.empty())
      {
        InvalidInput ex;
        ex.what = std::string("Invalid path: ") + path;
        throw ex;
      }
      
      _return = makeFileInfo(*res.begin());
    });
  }
  
  std::string ProjectServiceHandler::getFileNameFromPath(std::string path)
  {
    std::string name;  
    std::istringstream ss(path);    
    while(std::getline(ss, name, '/'));
    return name;
  }
  
  void ProjectServiceHandler::getFileStat(FileStat& _return, const FileId& file)
  {    
    SLog(util::DEBUG)  <<"getFileStat requested for file id:"<<file.fid;    
    FileInfo fileInfo;
    getFileInfo(fileInfo,file);    
    _return.info=fileInfo;    
    transaction ([&, this](){
        try
        {
          model::File f;
          
          if (!_db->find<model::File>(stoull(file.fid), f))
          {
            SLog(util::ERROR) << "invalid file ID: " << file.fid;
            
            InvalidId ex;
            ex.fid = file.fid;
            ex.what = "invalid file ID";
            throw ex;
          }
          f.content.load();
          if (f.content)
            _return.content=(f.content->content);
          
        }
        catch (odb::exception &odbex)
        {
          SLog(util::ERROR) << "datasource exception occured: " << odbex.what();
          
          DatasourceError ex;
          ex.what = odbex.what();
          throw ex;
        }        
    });    
  }
  
  void ProjectServiceHandler::getProjectInfo(ProjectInfo& _return, const ProjectId& project)
  {    
    ProjectInfo result;
    transaction ([&, this](){
        try
        {
          model::Project proj;
          
          if (!_db->find<model::Project>(project.prid, proj))
          {
            SLog(util::ERROR) << "invalid project ID: " << project.prid;
            
            InvalidId ex;
            ex.prid = project.prid;
            ex.what = "invalid project ID";
            throw ex;
          }
          
          typedef odb::query<model::Option> query;
          
          // TODO: It's not necessary to check if 'version' and 'paths' exists
          // in the database, because it's sure. The reason why we check them is
          // reverse compatibility with older databases which don't contain
          // these information.
          
          //--- Query version ---//
          
          std::string version = "";
          auto versionQuery = _db->query<model::Option>(query::key == "version");
          if (versionQuery.begin() != versionQuery.end())
            version = versionQuery.begin()->value;
          
          //--- Query paths ---//
          
          std::map<std::string, std::string> paths;
          auto pathsQuery = _db->query<model::Option>(query::key == "paths");
          
          if (pathsQuery.begin() != pathsQuery.end())
          {
            std::string pathsValue = pathsQuery.begin()->value + '|';

            while (!pathsValue.empty())
            {
              std::size_t pos  = pathsValue.find('|');
              std::string elem = pathsValue.substr(0, pos);

              pathsValue = pathsValue.substr(pos + 1);

              pos = elem.find(':');
              paths[elem.substr(0, pos)] = elem.substr(pos + 1);
            }
          }
          
          //--- Query name ---//
          
          result.project = project;
          result.name = _db->query<model::Option>(query::key == "name").begin()->value;
          result.version = version;
          result.paths = paths;
          result.timestamp = 0L;                            // TODO unknown
        }
        catch (odb::exception &odbex)
        {
          SLog(util::ERROR) << "datasource exception occured: " << odbex.what();
          
          DatasourceError ex;
          ex.what = odbex.what();
          throw ex;
        }        
    });
    std::swap(_return, result);
  }
  
  void ProjectServiceHandler::getRootFiles(std::vector<FileInfo> & _return, const ProjectId& project)
  {      
    SLog(util::DEBUG)  <<"getRootFiles requested ";
    transaction ([&, this](){
        typedef odb::result<model::File> fileResult;
        typedef odb::query<model::File> query;
        try{
          fileResult r (_db->query<model::File> (query::parent.is_null() && query::project==project.prid));
          model::File f;
          for (fileResult::iterator i(r.begin()); i!=r.end();++i){    
            i.load(f);
            if (f.path != "."){
              SLog(util::DEBUG)  <<"getRootFiles requested fileName:"<<f.path<<"fid:"<<f.id;              
              FileInfo fileInfo=makeFileInfo(f);   
              _return.push_back(fileInfo);
            }
          }
        } catch (odb::exception &odbex){
          SLog(util::ERROR) << "datasource exception occured: " << odbex.what();
          
          DatasourceError ex;
          ex.what = odbex.what();
          throw ex;
        }        
    });    
  }
  
  bool ProjectServiceHandler::fileInfoOrder(const FileInfo& left, const FileInfo& right)
  {
    if (left.isDirectory == right.isDirectory)
      return left.name < right.name;
    else
      return left.isDirectory;
  }
  
  void ProjectServiceHandler::getChildFiles(std::vector<FileInfo> & _return, const FileId& file)
  {
    typedef odb::result<model::File> fileResult;
    typedef odb::query<model::File> query;
    
    SLog(util::DEBUG)  <<"getChildfFiles requested ";
    
    transaction ([&, this](){
        try{
          fileResult r (_db->query<model::File> (query::parent==stoull(file.fid)));
          model::File f;
          for (fileResult::iterator i(r.begin()); i!=r.end();++i){    
            i.load(f);
            SLog(util::DEBUG)  <<"child filePath:"<<f.path;
            FileInfo fileInfo=makeFileInfo(f);
            _return.push_back(fileInfo); 
          }
        }
        catch (odb::exception &odbex){  
          SLog(util::ERROR) << "datasource exception occured: " << odbex.what();
          
          DatasourceError ex;
          ex.what = odbex.what();
          throw ex;
        }       
    });
    
    std::sort(_return.begin(), _return.end(), fileInfoOrder);
  }
  
  FileInfo ProjectServiceHandler::makeFileInfo(model::File &f)
  {    
    FileInfo fileInfo;
    fileInfo.file.fid=std::to_string(f.id);
    //parsing filename
    fileInfo.name=getFileNameFromPath(f.path);
    fileInfo.path=f.path;        
    
    if (f.type==model::File::Type::Directory)
    {
      SLog(util::DEBUG)  <<"this is a directory";
      fileInfo.isDirectory=true;      
    }
    else
    {
      fileInfo.isDirectory=false;
      fileInfo.hasChildren=false;
    }
    
    fileInfo.type = static_cast<FileType::type>(f.type);
    fileInfo.isGenerated=false;//what to put here?
    f.parent.load();
    
    if (f.parent)
      fileInfo.parent.fid=std::to_string(f.parent->id);
    
    switch (f.parseStatus)
    {
    case model::File::PSNone:
      fileInfo.parseStatus = FileParseStatus::Nothing;
      break;
    case model::File::PSPartiallyParsed:
      fileInfo.parseStatus = FileParseStatus::PartiallyParsed;
      break;
    case model::File::PSFullyParsed:
      fileInfo.parseStatus = FileParseStatus::FullyParsed;
      break;
    default:
      SLog(util::ERROR) << "Unknown parse status!";
      fileInfo.parseStatus = FileParseStatus::Nothing;
      break;
    }

    if (fileInfo.parseStatus == FileParseStatus::Nothing &&
        f.inSearchIndex)
    {
      fileInfo.parseStatus = FileParseStatus::OnlyInSearchIndex;
    }
    
    return fileInfo;
  }
  
  void ProjectServiceHandler::getSubtree(std::vector<FileInfo> & _return,
    const  FileId& file)
  {
    std::vector<FileInfo> rootFiles;
    getChildFiles(rootFiles,file);
    for (FileInfo f:rootFiles){
      _return.push_back(f);
      if (f.isDirectory){        
        getSubtree(_return,f.file);
      }
    }
  }
  
  void ProjectServiceHandler::getOpenTreeTillFile(std::vector<FileInfo>& _return, const FileId& file)
  {
    FileInfo fInfo;
    getFileInfo(fInfo, file);
    
    std::vector<std::string> pathParts;
    pathParts.push_back("");
    
    char* path = const_cast<char*>(fInfo.path.c_str());
    for (char* filename = std::strtok(path, "/"); filename; filename = std::strtok(0, "/"))
      pathParts.push_back(filename);
    
    ProjectId project;
    project.prid = 1;
    
    std::vector<FileInfo> currentChildren;
    
    getRootFiles(currentChildren, project);
    for (std::size_t i = 0; i < currentChildren.size(); ++i)
      _return.push_back(currentChildren[i]);
    
    for (const std::string& filename : pathParts)
      for (const FileInfo& child : currentChildren)
        if (child.name == filename)
        {
          FileId childFid = child.file;
      
          currentChildren.clear();
          getChildFiles(currentChildren, childFid);

          for (std::size_t i = 0; i < currentChildren.size(); ++i)
            _return.push_back(currentChildren[i]);
          
          break;
        }
  }
  
  void ProjectServiceHandler::getPathTillFile(std::vector<FileInfo> & _return, const  ::cc::service::core::FileId& file)
  {
    FileInfo fileInfo;
    getFileInfo(fileInfo, file);
    
    const std::string& path = fileInfo.path;
    
    std::size_t pos = -1;
    while ((pos = path.find('/', pos + 1)) != std::string::npos)
    {
      std::string p = path.substr(0, pos);
      if (p.empty())
        p = "/";
      
      FileInfo fileInfo;
      getFileInfoByPath(fileInfo, p);
      _return.push_back(fileInfo);
    }
    
    _return.push_back(fileInfo);
  }
  
  void ProjectServiceHandler::getBuildLog(std::vector<BuildLog> & _return, const  ::cc::service::core::FileId& file)
  {
    typedef odb::result<model::BuildLog> logResult;
    typedef odb::query<model::BuildLog> logQuery;
    
    transaction([&, this](){
      model::BuildLog mBuildLog;
      logResult res = _db->query<model::BuildLog>(logQuery::location.file == std::stoul(file.fid));
      
      for (logResult::iterator it = res.begin(); it != res.end(); ++it)
      {
        it.load(mBuildLog);
        
        BuildLog buildLog;
        buildLog.message = mBuildLog.log.message;
        
        switch (mBuildLog.log.type)
        {
          case model::BuildLogMessage::Unknown:    buildLog.messageType = MessageType::Unknown;    break;
          case model::BuildLogMessage::Error:      buildLog.messageType = MessageType::Error;      break;
          case model::BuildLogMessage::FatalError: buildLog.messageType = MessageType::FatalError; break;
          case model::BuildLogMessage::Warning:    buildLog.messageType = MessageType::Warning;    break;
          case model::BuildLogMessage::Note:       buildLog.messageType = MessageType::Note;       break;
          case model::BuildLogMessage::CodingRule: buildLog.messageType = MessageType::CodingRule; break;
        }
        
        buildLog.range.startpos.line   = mBuildLog.location.range.start.line;
        buildLog.range.startpos.column = mBuildLog.location.range.start.column;
        buildLog.range.endpos.line     = mBuildLog.location.range.end.line;
        buildLog.range.endpos.column   = mBuildLog.location.range.end.column;
        
        _return.push_back(buildLog);
      }
    });
  }
  
  void ProjectServiceHandler::getContainingFile(FileInfo& _return, const FileId& file)
  {
    std::shared_ptr<model::File> f=_db->load<model::File>(stoull(file.fid));
    FileId parent;
    parent.fid=std::to_string(f->parent->id);
    getFileInfo(_return,parent);
  }
  
  void ProjectServiceHandler::searchFile(std::vector<FileInfo> & _return, const std::string& searchText, const bool onlyFile) {
    SLog(util::DEBUG)  <<"searchFile requested "<<searchText;
    typedef odb::result<model::File> fileResult;
    typedef odb::query<model::File> query;
    transaction ([&, this](){
        try{
          std::string st = searchText;
          if (st.back() == '/')
            st.pop_back();
          
          fileResult r (onlyFile
              ? _db->query<model::File> (
                  query::type != model::File::Type::Directory &&
                  query::filename + SQL_ILIKE + query::_val ("%"+st+"%"))
              : _db->query<model::File> (query::path == st));

          model::File f;
          for (fileResult::iterator i(r.begin()); i!=r.end();++i){    
            i.load(f);
            SLog(util::DEBUG)  <<"search filePath:"<<f.path;
            FileInfo fileInfo=makeFileInfo(f);
            _return.push_back(fileInfo); 
          }
        }
        catch (odb::exception &odbex){  
          SLog(util::ERROR) << "datasource exception occured: " << odbex.what();
          
          DatasourceError ex;
          ex.what = odbex.what();
          throw ex;
        }       
    });        
    
  }

  void ProjectServiceHandler::checkDbAndThrow()
  {
    if (!_db)
    {
      SLog(util::ERROR) << "invalid database in workspace service";
      
      DatasourceError ex;
      ex.what = "invalid database";
      
      throw ex;
    }
    
    SLog(util::DEBUG) << "database is valid in workspace service";
  }
  
  void ProjectServiceHandler::getStatistics(
    std::vector<StatisticsInfo>& _return)
  {
    typedef odb::query<model::Statistics> SQ;

    std::map<std::string, int> summary;

    transaction ([&, this](){
      for (auto& stat : _db->query<model::Statistics>("ORDER BY" + SQ::group))
      {
        summary[stat.key] += stat.value;

        StatisticsInfo sinfo;

        sinfo.group = stat.group;
        sinfo.key   = stat.key;
        sinfo.value = stat.value;

        _return.push_back(std::move(sinfo));
      }
    });

    for (auto& stat : summary)
    {
      StatisticsInfo sinfo;

      sinfo.group = "Summary";
      sinfo.key   = stat.first;
      sinfo.value = stat.second;

      _return.push_back(std::move(sinfo));
    }
  }

} // core
} // service
} // cc
