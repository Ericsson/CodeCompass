#ifndef CODECOMPASS_PARSER_PROJECTPARSER_XMLPARSER_H
#define CODECOMPASS_PARSER_PROJECTPARSER_XMLPARSER_H

#include <vector>
#include <string>
#include <map>

namespace cc
{
namespace parser
{

class XMLParser
{
public:
  struct BuildAction
  {
    typedef std::vector<std::string> SCont;

    int         id;
    std::string type;
    std::string label;
    SCont       sources;
    SCont       targets;
    SCont       options;
  };

  struct BuildActions
  {
    std::vector<BuildAction> actions;

    std::map<std::string, std::string> roots;
  };

public:
  XMLParser();

  void setConfig(const std::string& configName_);
  bool parse(const std::string& xmlPath_, BuildActions& buildInfos_,
             std::map<std::string, std::string>& projectOptions_);
  
private:
  std::string _config;
};

} // parser
} // cc
#endif // CODECOMPASS_PARSER_PROJECTPARSER_XMLPARSER_H
