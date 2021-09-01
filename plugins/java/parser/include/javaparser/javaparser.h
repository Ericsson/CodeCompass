#ifndef CC_PARSER_JAVAPARSER_H
#define CC_PARSER_JAVAPARSER_H

#include <model/buildaction.h>

#include <parser/abstractparser.h>
#include <parser/parsercontext.h>

namespace cc
{
namespace parser
{

namespace fs = boost::filesystem;
namespace pr = boost::process;
namespace pt = boost::property_tree;

class JavaParser : public AbstractParser
{
public:
  JavaParser(ParserContext& ctx_);
  virtual ~JavaParser();
  virtual bool parse() override;

private:
  fs::path _java_path;
  bool accept(const std::string &path_);

  model::BuildActionPtr addBuildAction(
    const pt::ptree::value_type& command_);

  void addCompileCommand(
    const pt::ptree::value_type& command_,
    model::BuildActionPtr buildAction_,
    bool error_ = false);
};

} // parser
} // cc

#endif // CC_PARSER_JAVAPARSER_H
