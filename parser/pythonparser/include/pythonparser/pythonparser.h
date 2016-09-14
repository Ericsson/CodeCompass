#ifndef PYTHONPARSER_PYTHONPARSER_H
#define PYTHONPARSER_PYTHONPARSER_H

#include <string>
#include <memory>

#include <odb/nullable.hxx>

#include <parser/traversal.h>
#include <model/workspace.h>
#include <model/fileloc.h>
#include <pythonparser-api/pythonparserprocess.h>

namespace cc
{
namespace parser
{

class SourceManager;

class PythonParser :
  public Traversal,
  public PythonPersisterServiceIf,
  public std::enable_shared_from_this<PythonParser>
{
public:
  PythonParser(std::shared_ptr<model::Workspace> ws_);

public:
  virtual void beforeTraverse(
    const OptionMap& projectOptions_,
    SourceManager& srcMgr_) override;

  virtual void afterTraverse(SourceManager& srcMgr_) override;

  virtual DirIterCallback traverse(
    const std::string& path_,
    SourceManager& srcMgr_) override;

  virtual void stop() override;

  virtual void addNode(const TPythonAstNode& node_) override;
  virtual void addBinding(const TPythonBinding& binding_) override;
  virtual void addReference(const TPythonReference& reference_) override;
  virtual void addUnknown(const TPythonUnknown& unknown_) override;
  virtual void addClassDef(const TPythonClassDef& classDef_) override;
  virtual void addFunctionDef(const TPythonFunctionDef& functionDef_) override;
  virtual void addFunctionCall(const TPythonFunctionCall& functionCall_) override;
  virtual void addFunctionParam(const TPythonFunctionParam& functionParam) override;
  virtual void addDecorator(const TPythonDecorator& decorator_) override;
  virtual void addInheritance(const TPythonInheritance& inheritance_) override;
  virtual void addAttribute(const TPythonAttribute& attributes_) override;
  virtual void addVariable(const TPythonVariable& variable_) override;
  virtual void addVariableRef(const TPythonVariableRef& vref_) override;

  virtual void failedToParse(const std::string& filePath_) override;

private:
  model::FileLoc convertFileLoc(const TFileLoc& rhs_) const;
  odb::nullable<uint64_t> convertNullableWithHash(const std::string& rhs_) const;
  odb::nullable<std::string> convertNullable(const std::string& rhs_) const;

  /**
   * The database workspace object.
   */
  std::shared_ptr<model::Workspace> _workspace;

  std::unique_ptr<PytonhParserProcess> _parserProc;

  std::vector<std::string> _roots;

  SourceManager* _srcMgr;

  OptionMap _projectOptions;
};

} //parser
} //cc

#endif // PYTHONPARSER_PYTHONPARSER_H
