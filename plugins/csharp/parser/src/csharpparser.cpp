#include <csharpparser/csharpparser.h>

#include <boost/filesystem.hpp>

#include <util/logutil.h>

#include <memory>

namespace cc
{
namespace parser
{

CsharpParser::CsharpParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  _threadNum = _ctx.options["jobs"].as<int>();
}

bool CsharpParser::acceptCompileCommands(const std::string& path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".json";
}

bool CsharpParser::parse()
{        
  bool succes = true;

  for(std::string path : _ctx.options["input"].as<std::vector<std::string>>())
  {
    if(acceptCompileCommands(path))
    {
      LOG(info) << "CsharpParser parse path: " << path;
      succes = succes && parseCompileCommands(path);
    }
  }
  return true;
}

bool CsharpParser::parseCompileCommands(const std::string& path_) {
  pt::ptree _pt;
  pt::read_json(path_, _pt);
  pt::ptree _pt_filtered;
  pr::ipstream is;
  std::size_t file_index = 0;

  // Filter compile commands tree to contain only Csharp-related files
  std::copy_if (
    _pt.begin(),
    _pt.end(),
    std::back_inserter(_pt_filtered),
    [](pt::ptree::value_type& command_tree_)
    {
      auto ext =
        fs::extension(command_tree_.second.get<std::string>("file"));
      return ext == ".cs";
    });

  _numCompileCommands = _pt_filtered.size();

  //--- Process files ---//

  if (_numCompileCommands == 0) {
    LOG(info) << "Csharp-related compile commands not found in " << path_;
    return true;
  }

  //--- Create a thread pool for the current commands ---//
  std::unique_ptr<
    util::JobQueueThreadPool<ParseJob>> pool =
    util::make_thread_pool<ParseJob>(
      threadNum_, [this](ParseJob& job_)
      {
        const clang::tooling::CompileCommand& command = job_.command;

        LOG(info)
          << '(' << job_.index << '/' << _numCompileCommands << ')'
          << " Parsing " << command.Filename;

        int error = this->parseWorker(command);

        if (error)
          LOG(warning)
            << '(' << job_.index << '/' << _numCompileCommands << ')'
            << " Parsing " << command.Filename << " has been failed.";
      });
}

int CppParser::parseWorker(const clang::tooling::CompileCommand& command_)
{
  
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
