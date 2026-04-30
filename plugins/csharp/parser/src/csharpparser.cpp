#include <csharpparser/csharpparser.h>

#include <boost/filesystem.hpp>

#include <model/buildaction.h>
#include <model/buildaction-odb.hxx>
#include <model/buildsourcetarget.h>
#include <model/buildsourcetarget-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <parser/sourcemanager.h>

#include <util/logutil.h>
#include <util/odbtransaction.h>
#include <util/threadpool.h>
#include <util/filesystem.h>

#include <memory>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace cc
{
namespace parser
{

CsharpParser::CsharpParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  _threadNum = _ctx.options["jobs"].as<int>();
}

bool CsharpParser::acceptProjectBuildPath(const std::string& buildPath_)
{
  return fs::is_directory(buildPath_);
}

bool CsharpParser::parse()
{        
  bool success = true;

  std::vector<std::string> paths = _ctx.options["input"].as<std::vector<std::string>>();
  
    if (!paths.empty())     
    {
      LOG(debug) << "C# parser parse path: " << paths.size(); 
      success = success && parseProjectBuildPath(paths);
    }
    else
    {
      LOG(error) << "No input directories provided for C# parser!";
      success = false;
    }
  
  return success;
}

bool CsharpParser::parseProjectBuildPath(
  const std::vector<std::string>& paths_ //,
  ) 
{
  namespace ch = std::chrono;
  std::future<std::string> log;
  fs::path csharp_path = util::findCurrentExecutableDir() + "/../lib/csharp/";

  /*
   * Concatenate the command parameters to pass to the C# parser.
   * 1) C# parser binary
   * 2) Database connection string
   * 3) Project build directory path
   * 4) CC lib directory path
   * 5) Thread number
   * 6+) Source directories
   */
  std::string command("./CSharpParser ");
  command.append("'");
  command.append(_ctx.options["database"].as<std::string>());
  command.append("' '");
  command.append(csharp_path.string());
  command.append("' ");
  command.append(std::to_string(_ctx.options["jobs"].as<int>()));

  for (auto p : paths_)
  {
    if (fs::is_directory(p))
    {
      command.append(" '");
      command.append(p);
      command.append("' ");
    }
  }

  LOG(debug) << "CSharpParser command: " << command;

  ch::steady_clock::time_point begin = ch::steady_clock::now();
  
  int result = bp::system(command, bp::start_dir(csharp_path), bp::std_out > log);

  ch::steady_clock::time_point current = ch::steady_clock::now();
  float elapsed_time = ch::duration_cast<ch::milliseconds>(current - begin).count();
  LOG(debug) << "CSharp Parse time: " << elapsed_time << " ms";

  std::string line;
  std::stringstream log_str(log.get());
  int countFull = 0, countPart = 0;

  while(std::getline(log_str, line, '\n'))
  {
    // Skip empty lines or non-JSON lines (like debug info "ParallelRun ...")
    if (line.empty() || line[0] != '{') continue;

    try 
    {
      std::stringstream jsonStream(line);
      boost::property_tree::ptree pt;
      boost::property_tree::read_json(jsonStream, pt);

      bool fullyParsed = pt.get<bool>("fullyParsed");
      std::string filepath = pt.get<std::string>("filePath");
      std::string targetDll = pt.get<std::string>("targetDll");

      // Check if it's an error based on 'fullyParsed'
      bool isError = !fullyParsed;

      addSource(filepath, targetDll, isError);

      if (fullyParsed) { countFull++; }
      else { countPart++; }
    } 
    catch (const boost::property_tree::json_parser::json_parser_error& e) 
    {
      LOG(warning) << "Failed to parse JSON output from C# parser: " << e.what() << " | Line: " << line;
    }
    catch (const boost::property_tree::ptree_error& e)
    {
       LOG(warning) << "Missing expected JSON field from C# parser: " << e.what() << " | Line: " << line;
    }
  }

  ch::steady_clock::time_point after = ch::steady_clock::now();
  elapsed_time =
    ch::duration_cast<ch::milliseconds>(after - current).count();

  LOG(debug) << "C# source manage time: " << elapsed_time << " ms";
  LOG(info) << "Number of files fully parsed: " << countFull << 
    ", partially parsed: " << countPart << ", total: " <<  countFull+countPart;

  return result == 0;
}

void CsharpParser::addSource(const std::string& filepath_, const std::string& targetDll_, bool error_)
{
  util::OdbTransaction transaction(_ctx.db);

  model::BuildActionPtr buildAction(new model::BuildAction);
  buildAction->command = "dotnet build ";
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
}


CsharpParser::~CsharpParser()
{
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
extern "C"
{
  boost::program_options::options_description getOptions()
  {
    boost::program_options::options_description description("C# Plugin");

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
