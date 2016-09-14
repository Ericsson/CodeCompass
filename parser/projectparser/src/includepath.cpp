#include "includepath.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include <sstream>

namespace cc
{
namespace parser
{

#ifdef __unix__
  namespace _includepath_detail{
      /**
      * Executes shell command
      * returns the output of that command
      * returns "" on pipe error
      */
      std::string execute(const std::string& command_)
      {
          std::string result = "";
          FILE* pipe = popen(command_.c_str(), "r");
          {
              if (!pipe)
                  return "";
              while(!feof(pipe))
              {
                  char buffer[128];
                  if(fgets(buffer, 128, pipe) != NULL)
                      result += buffer;
              }
          }
          pclose(pipe);
          return result;
      }
  } // namespace _includepath_detail
#endif

  DefaultIncludePathFinder::SourceType
  DefaultIncludePathFinder::getTypeOf(std::string sourceFileName_) //not const& on intetion
  {
    if(sourceFileName_.substr(sourceFileName_.size()-3)==".ii")
    {
      sourceFileName_ = sourceFileName_.substr(0,sourceFileName_.size()-3);
    }
    std::string last2, last3, last4;
    last2 = sourceFileName_.substr(sourceFileName_.size()-2);
    last3 = sourceFileName_.substr(sourceFileName_.size()-3);
    last4 = sourceFileName_.substr(sourceFileName_.size()-4);
    if(last2 == ".C" || last2 == ".c")
    {
      return CSourceType;
    }
    if(last4 == ".CPP" || last4 == ".cpp"
       ||last4 == ".CXX" || last4 == ".cxx"
       ||last4 == ".C++" || last4 == ".c++"
       ||last3 == ".CC" || last3 == ".cc")
    {
      return CppSourceType;
    }
    return UnknownSourceType;
  }

   std::vector<std::string>
  DefaultIncludePathFinder::find(const std::string& sourceFileName_, const std::string& prefix_)
  {
    std::vector<std::string> result;
#ifdef __unix__
    SourceType st = getTypeOf(sourceFileName_);
    std::string command;
    switch(st)
    {
      case CSourceType:
        command = "cat /dev/null | cpp -x c -Wp,-v 2>&1";
        break;
      case CppSourceType:
        command = "cat /dev/null | cpp -x c++ -Wp,-v 2>&1";
        break;
      default:
        return result;
    }
    std::string output = _includepath_detail::execute(command);
    std::stringstream ss(output);
    std::string line;
    while(std::getline(ss, line) && line != "#include <...> search starts here:")
      ;
    while(std::getline(ss, line) && line != "End of search list.")
      result.push_back(prefix_+line.substr(1));
#endif
    return result;
  }

  
} // parser
} // cc