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
    if (line[0] == '+' || line[0] == '-')
    {
      std::string content = line.substr(1); // We cut off the +/- sign
      
      // Find the line (|) that separates the file and the DLL
      size_t separatorPos = content.find('|'); 
      
      if (separatorPos != std::string::npos) {
          // If it exists, we split the text along |
          std::string filepath = content.substr(0, separatorPos);
          std::string targetDll = content.substr(separatorPos + 1);
          
          // We clean up the spaces from the beginning
          filepath.erase(0, filepath.find_first_not_of(" \t"));
          
          addSource(filepath, targetDll, line[0] == '-'); 
      }
      else {
          // Fallback if for some reason the DLL name was not sent by C#
          content.erase(0, content.find_first_not_of(" \t"));
          addSource(content, "Unknown.dll", line[0] == '-');
      }

      if (line[0] == '+') { countFull++; }
      else { countPart++; }
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
  buildAction->command = "dotnet build " + targetDll_;  //buildAction->command = " ";
  buildAction->type = model::BuildAction::Compile;

  model::BuildSource buildSource;
  buildSource.file = _ctx.srcMgr.getFile(filepath_);
  buildSource.file->parseStatus = error_
    ? model::File::PSPartiallyParsed
    : model::File::PSFullyParsed;
  buildSource.file->type = "CS";
  buildSource.action = buildAction;


  model::BuildTarget buildTarget;
  buildTarget.action = buildAction;

  buildTarget.file = _ctx.srcMgr.getFile(targetDll_); 
  buildTarget.file->type = "CS_DLL";

  _ctx.srcMgr.updateFile(*buildSource.file);
  _ctx.srcMgr.updateFile(*buildTarget.file);
  _ctx.srcMgr.persistFiles();

  transaction([&, this] { 
    _ctx.db->persist(buildAction);
    _ctx.db->persist(buildSource);
    _ctx.db->persist(buildTarget); //new!!
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
