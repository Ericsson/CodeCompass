#include <csharpparser/csharpparser.h>

#include <boost/filesystem.hpp>

#include <model/buildaction.h>
#include <model/buildaction-odb.hxx>
#include <model/buildsourcetarget.h>
#include <model/buildsourcetarget-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <parser/sourcemanager.h>

#include <util/hash.h>
#include <util/logutil.h>
#include <util/odbtransaction.h>
#include <util/threadpool.h>

#include <memory>

namespace cc
{
namespace parser
{

CsharpParser::CsharpParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  _threadNum = _ctx.options["jobs"].as<int>();
}

bool CsharpParser::acceptProjectBuildPath(const std::vector<std::string>& path_)
{
  return path_.size()==2 && fs::is_directory(path_[0]) && fs::is_directory(path_[1]);
}

bool CsharpParser::parse()
{        
  bool succes = true;

  std::vector<std::string> paths = _ctx.options["input"].as<std::vector<std::string>>();
  
    if(acceptProjectBuildPath(paths))
    {
      LOG(info) << "CsharpParser parse path: " << paths[0];
      LOG(info) << "Parsed csharp project build path: " << paths[1];
      succes = succes && parseProjectBuildPath(paths);
    } else {
      LOG(info) << "Bulid path must be a directory!";
      succes = false;
    }
  
  return succes;
}

bool CsharpParser::parseProjectBuildPath(const std::vector<std::string>& paths_) {
  namespace chrono = std::chrono;
  fs::path csharp_path = fs::system_complete("../lib/csharp/");

  std::future<std::string> log;

  std::string command("./CSharpParser ");
  command.append("'");
  command.append(_ctx.options["database"].as<std::string>());
  command.append("' '");
  command.append(paths_[0]);
  command.append("' '");
  command.append(paths_[1]);
  command.append("' ");
  command.append(std::to_string(_ctx.options["jobs"].as<int>()));
  LOG(info) << "CSharpParser command: " << command;

  chrono::steady_clock::time_point begin = chrono::steady_clock::now();
  
  int result = bp::system(command, bp::start_dir(csharp_path), bp::std_out > log);

  chrono::steady_clock::time_point current = chrono::steady_clock::now();
  float elapsed_time =
    chrono::duration_cast<chrono::milliseconds>(current - begin).count();
  LOG(info) << "CSharp Parse time: " << elapsed_time << " ms";

  std::string line;
  bool error = false;

  std::stringstream log_str(log.get());
  
  while(std::getline(log_str, line, '\n')){
    if (line[0] != '/') {
      error = true;
    } else {
      addSource(line, error);
      //LOG(info) << line << (error ? " with errors" : "");
      error = false;
    }
  }
  chrono::steady_clock::time_point after = chrono::steady_clock::now();
  elapsed_time =
    chrono::duration_cast<chrono::milliseconds>(after - current).count();
  LOG(info) << "CSharp source manage time: " << elapsed_time << " ms";

  return result == 0;
}

void CsharpParser::addSource(const std::string& filepath_, bool error_){
  ///*
  util::OdbTransaction transaction(_ctx.db);

  model::BuildActionPtr buildAction(new model::BuildAction);
  buildAction->command = " ";
  buildAction->type = model::BuildAction::Compile;

  model::BuildSource buildSource;
  buildSource.file = _ctx.srcMgr.getFile(filepath_);
  buildSource.file->parseStatus = error_
    ? model::File::PSPartiallyParsed
    : model::File::PSFullyParsed;
  buildSource.file->type = "CS";
  buildSource.action = buildAction;

  _ctx.srcMgr.updateFile(*buildSource.file);
  _ctx.srcMgr.persistFiles();

  transaction([&, this] { 
    _ctx.db->persist(buildAction);
    _ctx.db->persist(buildSource);
  });
  //*/
}


CsharpParser::~CsharpParser()
{
}

/* These two methods are used by the plugin manager to allow dynamic loading
   of CodeCompass Parser plugins. Clang (>= version 6.0) gives a warning that
   these C-linkage specified methods return types that are not proper from a
   C code.

   These codes are NOT to be called from any C code. The C linkage is used to
   turn off the name mangling so that the dynamic loader can easily find the
   symbol table needed to set the plugin up.
*/
// When writing a plugin, please do NOT copy this notice to your code.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("Dummy Plugin");

    description.add_options()
        ("dummy-arg", po::value<std::string>()->default_value("Dummy arg"),
          "This argument will be used by the dummy parser.");

    return description;
  }

  std::shared_ptr<CsharpParser> make(ParserContext& ctx_)
  {
    return std::make_shared<CsharpParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
