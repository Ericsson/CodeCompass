#ifndef CC_PARSER_COMPILATIONDATABASEPARSER_H
#define CC_PARSER_COMPILATIONDATABASEPARSER_H

#include <parser/abstract_parser.h>
#include <parser/parser_context.h>

#include <model/buildaction.h>
#include <model/buildsource.h>
#include <model/buildtarget.h>

#include <odb/core.hxx>

namespace cc
{
namespace parser
{
  
class CompilationDatabaseParser : public AbstractParser
{
public:
  CompilationDatabaseParser(ParserContext& ctx_);
  virtual ~CompilationDatabaseParser();  
  virtual std::string getName() const override;
  virtual std::vector<std::string> getDependentParsers() const override;
  virtual bool parse() override; 
private:
  /**
   * A build command.
   */
  struct BuildCommand
  {
    /**
     * Argument vector including the compiler command (first argument).
     */
    std::vector<std::string> arguments;
    /**
     * Source to target map.
     */
    std::map<std::string, std::string> sourceToTarget;
  };
  
  /**
   * Build action id -> BuildCommand map type.
   */
  using BuildActions = std::map<
      std::uint64_t,
      BuildCommand
    >;
  
  bool accept(const std::string& path_);
  bool initParse(const std::string& path_);
  
  void createNewAction( const BuildActions::value_type& jsonAction_);

  model::BuildActionPtr addBuildAction(
    model::BuildAction::Type type_,
    uint64_t id_,
    const std::string& command_);

  model::BuildSourcePtr addBuildSource(
    model::BuildActionPtr action_,
    const std::string& path_);

  model::BuildTargetPtr addBuildTarget(
    model::BuildActionPtr action_,
    const std::string& path_);
  
  /**
   * Build action id -> BuildCommand map.
   */
  BuildActions _actions;

  std::vector<model::BuildSourcePtr> _buildSources;
  std::vector<model::BuildTargetPtr> _buildTargets;

  std::shared_ptr<odb::database> _db;
};
  
} // parser
} // cc

#endif // CC_PARSER_COMPILATIONDATABASEPARSER_H
