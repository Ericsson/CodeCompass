#include <haskellparser/haskellparser.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/join.hpp>

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendAction.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>

#include <util/logutil.h>

#include <memory>

namespace cc
{
namespace parser
{

HaskellParser::HaskellParser(ParserContext& ctx_): AbstractParser(ctx_)
{
}

std::vector<std::string> HaskellParser::getDependentParsers()  const
{
  return std::vector<std::string>{};
}

bool HaskellParser::accept(const std::string& path_)
{
  std::string ext = boost::filesystem::extension(path_);
  return ext == ".hs";
}

bool HaskellParser::parse()
{
	//for (const auto& it : _ctx.options) {
	//	LOG(info) << it.first.c_str();
	//}

  for (const std::string& input
    : _ctx.options["input"].as<std::vector<std::string>>())
    if (boost::filesystem::is_regular_file(input)) {
      LOG(info) << "HaskellParser parse path: " << input;
      parseByJson(input);
    }
  return true;
}


bool HaskellParser::parseByJson(
  const std::string& jsonFile_)
{
  std::string errorMsg;
  std::unique_ptr<clang::tooling::JSONCompilationDatabase> compDb
    = clang::tooling::JSONCompilationDatabase::loadFromFile(
        jsonFile_, errorMsg,
        clang::tooling::JSONCommandLineSyntax::Gnu);
  if (!errorMsg.empty())
  {
    LOG(error) << errorMsg;
    return false;
  }

  std::vector<clang::tooling::CompileCommand> compileCommands =
    compDb->getAllCompileCommands();
  //std::size_t numCompileCommands = compileCommands.size();
  std::string dbConnStr = _ctx.options["database"].as<std::string>();

  LOG(info) << "HaskellParser replicating build commands.";
  for (const auto& command : compileCommands)
  {
	  std::vector<std::string> compComm = command.CommandLine;
	  compComm.push_back("-fforce-recomp");
	  compComm.push_back("-fplugin");
	  compComm.push_back("Development.CodeCompass.Parser.Plugin");
	  compComm.push_back("-fplugin-opt");
	  compComm.push_back("Development.CodeCompass.Parser.Plugin:" + dbConnStr + "");
	  std::string cm = boost::algorithm::join(compComm, std::string(" "));
	  LOG(info) << "Performing " << cm << " in " << command.Directory;
	  
	  std::string program = compComm[0];
	  char **ptrs = new char*[compComm.size() + 1];
	  for (size_t i = 0; i < compComm.size(); ++i) {
		  char* cstr = new char[compComm[i].length() + 1];
		  std::strcpy(cstr, compComm[i].c_str());
		  ptrs[i] = cstr;
	  }
	  ptrs[compComm.size()] = NULL;
	  execv(program.c_str(), ptrs);
    
    
    //bp::child c(command.Directory, command.CommandLine, std_out > output_stream, std_err > error_stream);
    
    //c.wait();
    
    //std::string line;
    
    //while (output_stream && std::getline(output_stream, line) && !line.empty()) {
		//LOG(info) << line;
	//}    
    //while (error_stream && std::getline(error_stream, line) && !line.empty()) {
		//LOG(error) << line;
	//}

	
  }

  return true;
}

HaskellParser::~HaskellParser()
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
    boost::program_options::options_description description("Haskell Plugin");

    /*description.add_options()
        ("dummy-arg", po::value<std::string>()->default_value("Dummy arg"),
          "This argument will be used by the dummy parser.");*/

    return description;
  }

  std::shared_ptr<HaskellParser> make(ParserContext& ctx_)
  {
    return std::make_shared<HaskellParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
