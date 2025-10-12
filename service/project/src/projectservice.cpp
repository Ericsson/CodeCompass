#include <algorithm>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <odb/query.hxx>

#include <model/file.h>
#include <model/file-odb.hxx>
#include <model/buildlog.h>
#include <model/buildlog-odb.hxx>
#include <model/statistics.h>
#include <model/statistics-odb.hxx>

#include <util/dbutil.h>
#include <util/odbtransaction.h>

#include <projectservice/projectservice.h>

namespace cc
{
namespace service
{
namespace core
{

ProjectServiceHandler::ProjectServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& /*context_*/)
    : _db(db_), _transaction(db_), _datadir(*datadir_)
{
}

void ProjectServiceHandler::getFileInfo(
  FileInfo& return_,
  const FileId& fileId_)
{
  _transaction([&, this](){
    model::File f;

    if (!_db->find(std::stoull(fileId_), f))
    {
      InvalidId ex;
      ex.__set_fid(fileId_);
      ex.__set_msg("Invalid file ID");
      throw ex;
    }

    return_ = makeFileInfo(f);
  });
}

void ProjectServiceHandler::getFileInfoByPath(
  FileInfo& return_,
  const std::string& path_)
{
  _transaction([&, this]() {
    typedef odb::query<model::File> FileQuery;
    typedef odb::result<model::File> FileResult;

    FileResult res = _db->query<model::File>(FileQuery::path == path_);

    if (res.empty())
    {
      InvalidInput ex;
      ex.__set_msg("Invalid path: " + path_);
      throw ex;
    }

    return_ = makeFileInfo(*res.begin());
  });
}

void ProjectServiceHandler::getFileContent(
  std::string& return_,
  const FileId& fileId_)
{
  _transaction([&, this](){
    model::File f;

    if (!_db->find(std::stoull(fileId_), f))
    {
      InvalidId ex;
      ex.__set_msg("Invalid file ID: ");
      ex.__set_fid(fileId_);
      throw ex;
    }

    if(std::shared_ptr<model::FileContent> fileContent = f.content.load())
      return_ = fileContent->content;
  });
}

void ProjectServiceHandler::getParent(
  FileInfo& return_,
  const FileId& fileId_)
{
  model::FilePtr f = _db->load<model::File>(std::stoull(fileId_));

  if (!f)
  {
    InvalidId ex;
    ex.__set_msg("Invalid file ID");
    ex.__set_fid(fileId_);
    throw ex;
  }

  getFileInfo(return_, std::to_string(f->parent.object_id()));
}

void ProjectServiceHandler::getRootFiles(std::vector<FileInfo>& return_)
{
  _transaction([&, this](){
    typedef odb::result<model::File> FileResult;
    typedef odb::query<model::File> FileQuery;

    FileResult r = _db->query<model::File>(FileQuery::parent.is_null());
    model::File f;

    for (FileResult::iterator i = r.begin(); i != r.end(); ++i)
    {
      i.load(f);
      return_.push_back(makeFileInfo(f));
    }
  });

  std::sort(return_.begin(), return_.end(), fileInfoOrder);
}

void ProjectServiceHandler::getChildFiles(
  std::vector<FileInfo>& return_,
  const FileId& fileId_)
{
  typedef odb::result<model::File> FileResult;
  typedef odb::query<model::File> FileQuery;

  _transaction([&, this](){
    FileResult r
      = _db->query<model::File>(FileQuery::parent == std::stoull(fileId_));

    model::File f;

    for (FileResult::iterator i = r.begin(); i != r.end(); ++i)
    {
      i.load(f);
      return_.push_back(makeFileInfo(f));
    }
  });

  std::sort(return_.begin(), return_.end(), fileInfoOrder);
}

void ProjectServiceHandler::getSubtree(
  std::vector<FileInfo>& return_,
  const FileId& fileId_)
{
  std::vector<FileInfo> childFiles;
  getChildFiles(childFiles, fileId_);

  for (const FileInfo& f : childFiles)
  {
    return_.push_back(f);
    if (f.isDirectory)
      getSubtree(return_, f.id);
  }
}

void ProjectServiceHandler::getOpenTreeTillFile(
  std::vector<FileInfo>& return_,
  const FileId& fileId_)
{
  FileInfo fInfo;
  getFileInfo(fInfo, fileId_);

  std::vector<std::string> pathParts;

  char* path = const_cast<char*>(fInfo.path.c_str());
  for (char* filename = std::strtok(path, "/");
       filename;
       filename = std::strtok(0, "/"))
    pathParts.push_back(filename);

  std::vector<FileInfo> sub;
  getRootFiles(sub);
  std::copy(sub.begin(), sub.end(), std::back_inserter(return_));

  for (const std::string& filename : pathParts)
    for (const FileInfo& child : sub)
      if (child.name == filename)
      {
        sub.clear();
        getChildFiles(sub, child.id);
        std::copy(sub.begin(), sub.end(), std::back_inserter(return_));
        break;
      }
}

void ProjectServiceHandler::getPathTillFile(
  std::vector<FileInfo>& return_,
  const FileId& fileId_)
{
  FileInfo fileInfo;
  getFileInfo(fileInfo, fileId_);

  const std::string& path = fileInfo.path;

  std::size_t pos = -1;
  while ((pos = path.find('/', pos + 1)) != std::string::npos)
  {
    std::string p = path.substr(0, pos);
    if (p.empty())
      p = "/";

    FileInfo fileInfo;
    getFileInfoByPath(fileInfo, p);
    return_.push_back(fileInfo);
  }

  return_.push_back(fileInfo);
}

void ProjectServiceHandler::getBuildLog(
  std::vector<BuildLog>& return_,
  const FileId& fileId_)
{
  typedef odb::result<model::BuildLog> LogResult;
  typedef odb::query<model::BuildLog> LogQuery;

  _transaction([&, this](){
    model::BuildLog mBuildLog;
    LogResult res = _db->query<model::BuildLog>(
      LogQuery::location.file == std::stoull(fileId_));

    for (LogResult::iterator it = res.begin(); it != res.end(); ++it)
    {
      it.load(mBuildLog);

      BuildLog buildLog;
      buildLog.__set_message(mBuildLog.log.message);

      switch (mBuildLog.log.type)
      {
        case model::BuildLogMessage::Unknown:
          buildLog.__set_messageType(MessageType::Unknown);
          break;
        case model::BuildLogMessage::Error:
          buildLog.__set_messageType(MessageType::Error);
          break;
        case model::BuildLogMessage::FatalError:
          buildLog.__set_messageType(MessageType::FatalError);
          break;
        case model::BuildLogMessage::Warning:
          buildLog.__set_messageType(MessageType::Warning);
          break;
        case model::BuildLogMessage::Note:
          buildLog.__set_messageType(MessageType::Note);
          break;
        case model::BuildLogMessage::CodingRule:
          buildLog.__set_messageType(MessageType::CodingRule);
          break;
      }

      buildLog.range.startpos.line   = mBuildLog.location.range.start.line;
      buildLog.range.startpos.column = mBuildLog.location.range.start.column;
      buildLog.range.endpos.line     = mBuildLog.location.range.end.line;
      buildLog.range.endpos.column   = mBuildLog.location.range.end.column;

      return_.push_back(buildLog);
    }
  });
}

void ProjectServiceHandler::searchFile(
  std::vector<FileInfo>& return_,
  const std::string& text_,
  const bool onlyFile_)
{
  typedef odb::result<model::File> FileResult;
  typedef odb::query<model::File> FileQuery;

  std::string text = text_;

  _transaction([&, this](){
    if (text_.back() == '/')
      text.pop_back();

    FileResult r
      = onlyFile_
      ? _db->query<model::File>(
          FileQuery::type != model::File::DIRECTORY_TYPE &&
          FileQuery::filename + SQL_ILIKE + FileQuery::_val("%" + text + "%"))
      : _db->query<model::File>(
          FileQuery::path + SQL_ILIKE + FileQuery::_val("%" + text + "%"));

    model::File f;
    for (FileResult::iterator i = r.begin(); i != r.end(); ++i)
    {
      i.load(f);
      return_.push_back(makeFileInfo(f));
    }
  });
}

void ProjectServiceHandler::getStatistics(
  std::vector<StatisticsInfo>& return_)
{
  typedef odb::query<model::Statistics> SQ;

  std::map<std::string, int> summary;

  _transaction([&, this](){
    for (const auto& stat :
         _db->query<model::Statistics>("ORDER BY" + SQ::group))
    {
      summary[stat.key] += stat.value;

      StatisticsInfo sinfo;

      sinfo.__set_group(stat.group);
      sinfo.__set_key(stat.key);
      sinfo.__set_value(stat.value);

      return_.push_back(std::move(sinfo));
    }
  });

  for (const auto& stat : summary)
  {
    StatisticsInfo sinfo;

    sinfo.__set_group("Summary");
    sinfo.__set_key(stat.first);
    sinfo.__set_value(stat.second);

    return_.push_back(std::move(sinfo));
  }
}

void ProjectServiceHandler::getFileTypes(
  std::vector<std::string>& return_)
{
  typedef odb::result<model::FileTypeView> FileTypeResult;

  _transaction([&, this](){
    FileTypeResult result = _db->query<model::FileTypeView>();
    std::transform(
      result.begin(),
      result.end(),
      std::back_inserter(return_),
      [](const model::FileTypeView& view) { return view.type; });
  });
}

void ProjectServiceHandler::getLabels(
  std::map<std::string, std::string>& return_)
{
  namespace pt = boost::property_tree;

  const std::string projInfo = _datadir + "/project_info.json";

  pt::ptree root;
  pt::read_json(projInfo, root);

  if (boost::optional<pt::ptree&> labels = root.get_child_optional("labels"))
    for (const pt::ptree::value_type& label : *labels)
      return_[label.first] = label.second.data();
}

FileInfo ProjectServiceHandler::makeFileInfo(model::File& f)
{
  FileInfo fileInfo;

  fileInfo.__set_id(std::to_string(f.id));
  fileInfo.__set_name(f.filename);
  fileInfo.__set_path(f.path);
  
  if (f.type == model::File::DIRECTORY_TYPE)
  {
    fileInfo.__set_isDirectory(true);
  }
  else
  {
    fileInfo.__set_isDirectory(false);
    fileInfo.__set_hasChildren(false); // TODO: Why false?
  }

  fileInfo.__set_type(f.type);

  if (f.parent)
    fileInfo.__set_parent(std::to_string(f.parent.object_id()));

  switch (f.parseStatus)
  {
    default:
    case model::File::PSNone:
      fileInfo.__set_parseStatus(FileParseStatus::Nothing);
      break;
    case model::File::PSPartiallyParsed:
      fileInfo.__set_parseStatus(FileParseStatus::PartiallyParsed);
      break;
    case model::File::PSFullyParsed:
      fileInfo.__set_parseStatus(FileParseStatus::FullyParsed);
      break;
  }

  if (fileInfo.parseStatus == FileParseStatus::Nothing && f.inSearchIndex)
    fileInfo.__set_parseStatus(FileParseStatus::OnlyInSearchIndex);

  return fileInfo;
}

bool ProjectServiceHandler::fileInfoOrder(
  const FileInfo& left_,
  const FileInfo& right_)
{
  if (left_.isDirectory == right_.isDirectory)
    return left_.name < right_.name;
  else
    return left_.isDirectory;
}

} // core
} // service
} // cc
