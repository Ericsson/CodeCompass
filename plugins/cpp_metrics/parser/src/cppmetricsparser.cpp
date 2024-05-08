#include <cppmetricsparser/cppmetricsparser.h>

#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>
#include <model/cppcohesionmetrics.h>
#include <model/cppcohesionmetrics-odb.hxx>
#include <model/cpprelationalcohesion.h>
#include <model/cpprelationalcohesion-odb.hxx>
#include <model/cppfilemetrics.h>
#include <model/cppfilemetrics-odb.hxx>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>

#include <boost/filesystem.hpp>

#include <util/filesystem.h>
#include <util/logutil.h>
#include <util/filesystem.h>
#include <util/odbtransaction.h>

#include <memory>
#include <iostream>

namespace cc
{
namespace parser
{

namespace fs = boost::filesystem;

CppMetricsParser::CppMetricsParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  for (const std::string& path :
    _ctx.options["input"].as<std::vector<std::string>>())
    _inputPaths.push_back(fs::canonical(path).string());

  util::OdbTransaction {_ctx.db} ([&, this] {
    for (const model::CppFileMetrics& fm
      : _ctx.db->query<model::CppFileMetrics>())
    {
      _fileIdCache.insert(fm.file);
    }

    for (const model::CppAstNodeMetricsFileView& anm
      : _ctx.db->query<model::CppAstNodeMetricsFileView>())
    {
      _astNodeIdCache.emplace(anm.astNodeId, anm.fileId);
    }
  });
}

bool CppMetricsParser::cleanupDatabase()
{
  if (!_fileIdCache.empty())
  {
    try
    {
      util::OdbTransaction {_ctx.db} ([this] {
        for (const model::File& file
          : _ctx.db->query<model::File>(
          odb::query<model::File>::id.in_range(_fileIdCache.begin(), _fileIdCache.end())))
        {
          auto it = _ctx.fileStatus.find(file.path);
          if (it != _ctx.fileStatus.end() &&
              (it->second == cc::parser::IncrementalStatus::DELETED ||
               it->second == cc::parser::IncrementalStatus::MODIFIED ||
               it->second == cc::parser::IncrementalStatus::ACTION_CHANGED))
          {
            LOG(info) << "[cxxmetricsparser] Database cleanup: " << file.path;

            _ctx.db->erase_query<model::CppFileMetrics>(odb::query<model::CppFileMetrics>::file == file.id);
            _fileIdCache.erase(file.id);
          }
        }

        for (const auto& pair : _astNodeIdCache)
        {
          auto file = _ctx.db->query_one<model::File>(
            odb::query<model::File>::id == pair.second);

          auto it = _ctx.fileStatus.find(file->path);
          if (it != _ctx.fileStatus.end() &&
              (it->second == cc::parser::IncrementalStatus::DELETED ||
               it->second == cc::parser::IncrementalStatus::MODIFIED ||
               it->second == cc::parser::IncrementalStatus::ACTION_CHANGED))
          {
            LOG(info) << "[cxxmetricsparser] Database cleanup: " << file->path;

            _ctx.db->erase_query<model::CppAstNodeMetrics>(odb::query<model::CppAstNodeMetrics>::astNodeId == pair.first);
            _astNodeIdCache.erase(pair.first);
          }
        }
      });
    }
    catch (odb::database_exception&)
    {
      LOG(fatal) << "Transaction failed in cxxmetrics parser!";
      return false;
    }
  }
  return true;
}

void CppMetricsParser::functionParameters()
{
  util::OdbTransaction {_ctx.db} ([&, this]
  {
    for (const model::CppFunctionParamCountWithId& paramCount
      : _ctx.db->query<model::CppFunctionParamCountWithId>())
    {
      // Skip functions that were included from external libraries.
      if (!cc::util::isRootedUnderAnyOf(_inputPaths, paramCount.filePath))
        continue;

      model::CppAstNodeMetrics funcParams;
      funcParams.astNodeId = paramCount.id;
      funcParams.type = model::CppAstNodeMetrics::Type::PARAMETER_COUNT;
      funcParams.value = paramCount.count;
      _ctx.db->persist(funcParams);
    }
  });
}

void CppMetricsParser::functionMcCabe()
{
  util::OdbTransaction {_ctx.db} ([&, this]
  {
    for (const model::CppFunctionMcCabe& function
      : _ctx.db->query<model::CppFunctionMcCabe>())
    {
      // Skip functions that were included from external libraries.
      if (!cc::util::isRootedUnderAnyOf(_inputPaths, function.filePath))
        continue;

      model::CppAstNodeMetrics funcMcCabe;
      funcMcCabe.astNodeId = function.astNodeId;
      funcMcCabe.type = model::CppAstNodeMetrics::Type::MCCABE;
      funcMcCabe.value = function.mccabe;
      _ctx.db->persist(funcMcCabe);
    }
  });
}

void CppMetricsParser::functionBumpyRoad()
{
  util::OdbTransaction {_ctx.db} ([&, this]
  {
    for (const model::CppFunctionBumpyRoad& function
      : _ctx.db->query<model::CppFunctionBumpyRoad>())
    {
      // Skip functions that were included from external libraries.
      if (!cc::util::isRootedUnderAnyOf(_inputPaths, function.filePath))
        continue;

      const double dB = function.bumpiness;
      const double dC = function.statementCount;
      const bool empty = function.statementCount == 0;

      model::CppAstNodeMetrics metrics;
      metrics.astNodeId = function.astNodeId;
      metrics.type = model::CppAstNodeMetrics::Type::BUMPY_ROAD;
      metrics.value = empty ? 1.0 : (dB / dC);
      _ctx.db->persist(metrics);
    }
  });
}

void CppMetricsParser::relationalCohesion()
{
  util::OdbTransaction {_ctx.db} ([&, this]
  {

    std::unordered_set<std::string> filepaths;
    std::vector<std::string> filepathVector;
    filepathVector.insert(filepathVector.end(), filepaths.begin(), filepaths.end());
    std::unordered_map<std::string, std::unordered_set<std::string>> pathsInModule;
    std::unordered_map<std::string, std::unordered_set<std::uint64_t>> typesUnderPath;
    std::unordered_map<std::string, std::unordered_set<std::uint64_t>> functionsUnderPath;
    std::unordered_map<std::uint64_t,std::string> typeDefinitionPaths;

    //get the project filepaths
    //change this later to only get the required modules
    for (const model::RelationalCohesionFileView& file
      : _ctx.db->query<model::RelationalCohesionFileView>())
    {
      if (cc::util::isRootedUnderAnyOf( _inputPaths, file.filePath) &&
      file.fileType == "DIR")
      {
        filepaths.insert(file.filePath);
      }
    }

    //get what filepaths are contained within a module
    for (auto &&currentPath : filepaths)
    {
      std::vector<std::string> pathVector;
      pathVector.push_back(currentPath);
      std::unordered_set<std::string> pathsFound;
      for (auto &&path : filepaths)
      {
        if (cc::util::isRootedUnderAnyOf(pathVector,path))
        {
          pathsFound.insert(path);
        }
      }
        pathsInModule.insert(std::make_pair(currentPath,pathsFound));
    }

    //find types defined under every filepath in the project
    for (auto &&currentPath : filepaths)
    {
      std::unordered_set<std::uint64_t> typesFound;
      std::vector<std::string> pathVector;
      pathVector.push_back(currentPath);
      for (const model::RelationalCohesionRecordView& record
      : _ctx.db->query<model::RelationalCohesionRecordView>())
      {
        if (cc::util::isRootedUnderAnyOf(pathVector,record.filePath))
        {
          typesFound.insert(record.typeHash);
        }
      }
      typesUnderPath.insert(std::make_pair(currentPath,typesFound));
    }

    //save where each type is defined so self relations can be excluded
    for (const model::RelationalCohesionRecordView& record
    : _ctx.db->query<model::RelationalCohesionRecordView>())
    {
      typeDefinitionPaths.insert(std::make_pair(record.typeHash,record.filePath));
    }
   
    //find relations
    for (auto &&path : filepaths) 
    {
      std::vector<std::string> currentModulePaths(
        pathsInModule[path].begin(), pathsInModule[path].end()
      );

      uint relationsInModule = 0; //R
      std::unordered_set<std::uint64_t> relationsInCurrentFile;
      std::unordered_set<std::string> filesAlreadyChecked;
      //select the next module
      for (auto &&modulePath : currentModulePaths)
      {
        std::vector<std::string> pathVector;
        pathVector.push_back(modulePath);
        //find the files defined under the current module member
        for (const model::RelationalCohesionFileView& file
        : _ctx.db->query<model::RelationalCohesionFileView>())
        {
          if (file.fileType != "CPP" && 
              !cc::util::isRootedUnderAnyOf(pathVector,file.filePath) &&
              filesAlreadyChecked.find(file.filePath) != filesAlreadyChecked.end()
          )
          {
            continue;
          }
          filesAlreadyChecked.insert(file.filePath);
          relationsInCurrentFile.clear();
          //check function return types
          for (const model::RelationalCohesionFunctionView& function
          : _ctx.db->query<model::RelationalCohesionFunctionView>())
          {
            if (
              function.filePath == file.filePath &&
              //check if function is in the current file
              typeDefinitionPaths[function.returnType] != function.filePath &&
              //check for self relation
              cc::util::isRootedUnderAnyOf(currentModulePaths,typeDefinitionPaths[function.returnType]) &&
              //check if return type is defined within the current module
              relationsInCurrentFile.find(function.returnType) == relationsInCurrentFile.end()
              //check if relation is already found
            )
            {
              ++relationsInModule;
              relationsInCurrentFile.insert(function.returnType);
            }
            
          }

          //check function parameters
          for (const model::RelationalCohessionFunctionParameterView& parameter
          : _ctx.db->query<model::RelationalCohessionFunctionParameterView>())
          {
            if (
              parameter.filePath == file.filePath &&
              //check if the parameter is in the current file
              typeDefinitionPaths[parameter.typeHash] != parameter.filePath &&
              //check for self relation
              cc::util::isRootedUnderAnyOf(currentModulePaths,typeDefinitionPaths[parameter.typeHash]) &&
              //check if the type is defined within the current module
              relationsInCurrentFile.find(parameter.typeHash) == relationsInCurrentFile.end()
              //check if relation is already found
            )
            {
              ++relationsInModule;
              relationsInCurrentFile.insert(parameter.typeHash);
            }
            
          }

          //check function locals
          for (const model::RelationalCohessionFunctionLocalView& local
          : _ctx.db->query<model::RelationalCohessionFunctionLocalView>())
          {
            if (
              local.filePath == file.filePath &&
              //check if the local variable is in the current file
              typeDefinitionPaths[local.typeHash] != local.filePath &&
              //check for self relation
              cc::util::isRootedUnderAnyOf(currentModulePaths,typeDefinitionPaths[local.typeHash]) &&
              //check if the type is defined within the current module
              relationsInCurrentFile.find(local.typeHash) == relationsInCurrentFile.end()
              //check if relation is already found
            )
            {
              ++relationsInModule;
              relationsInCurrentFile.insert(local.typeHash);
            }
            
          }

          //check variables
          for (const model::RelationalCohesionVariableView& variable
          : _ctx.db->query<model::RelationalCohesionVariableView>())
          {
            if (
              variable.filePath == file.filePath &&
              //check if the variable is in the current file
              typeDefinitionPaths[variable.typeHash] != variable.filePath &&
              //check for self relation
              cc::util::isRootedUnderAnyOf(currentModulePaths,typeDefinitionPaths[variable.typeHash]) &&
              //check if the type is defined within the current module
              relationsInCurrentFile.find(variable.typeHash) == relationsInCurrentFile.end()
              //check if relation is already found
            )
            {
              ++relationsInModule;
              relationsInCurrentFile.insert(variable.typeHash);
            }
            
          }

        }
        
      }
      //calculate relational cohesion for module
      //formula: H = (R + 1)/ N
      //where H is the relational cohesion value,
      //R is the number of relationships internal to the module,
      //N is the number of types in the module

      uint numberOfTypesInModule = typesUnderPath[path].size(); //N

      double relationalCohesion = 
        (static_cast<double>(relationsInModule) + 1.0) / static_cast<double>(numberOfTypesInModule);
      
      //TODO: persistance

    }
    

    
    
    
  });
}

void CppMetricsParser::lackOfCohesion()
{
  util::OdbTransaction {_ctx.db} ([&, this]
  {
    // Simplify some type names for readability.
    typedef std::uint64_t HashType;

    typedef odb::query<model::CohesionCppFieldView>::query_columns QField;
    const auto& QFieldTypeHash = QField::CppMemberType::typeHash;

    typedef odb::query<model::CohesionCppMethodView>::query_columns QMethod;
    const auto& QMethodTypeHash = QMethod::CppMemberType::typeHash;
    
    typedef odb::query<model::CohesionCppAstNodeView>::query_columns QNode;
    const auto& QNodeFilePath = QNode::File::path;
    const auto& QNodeRange = QNode::CppAstNode::location.range;

    // Calculate the cohesion metric for all types.
    for (const model::CohesionCppRecordView& type
      : _ctx.db->query<model::CohesionCppRecordView>())
    {
      // Skip types that were included from external libraries.
      if (!cc::util::isRootedUnderAnyOf(_inputPaths, type.filePath))
        continue;

      std::unordered_set<HashType> fieldHashes;
      // Query all fields of the current type.
      for (const model::CohesionCppFieldView& field
        : _ctx.db->query<model::CohesionCppFieldView>(
          QFieldTypeHash == type.entityHash
        ))
      {
        // Record these fields for later use.
        fieldHashes.insert(field.entityHash);
      }
      std::size_t fieldCount = fieldHashes.size();

      std::size_t methodCount = 0;
      std::size_t totalCohesion = 0;
      // Query all methods of the current type.
      for (const model::CohesionCppMethodView& method
        : _ctx.db->query<model::CohesionCppMethodView>(
          QMethodTypeHash == type.entityHash
        ))
      {
        // Do not consider methods with no explicit bodies.
        const model::Position start(method.startLine, method.startColumn);
        const model::Position end(method.endLine, method.endColumn);
        if (start < end)
        {
          std::unordered_set<HashType> usedFields;
          
          // Query all AST nodes that use a variable for reading or writing...
          for (const model::CohesionCppAstNodeView& node
            : _ctx.db->query<model::CohesionCppAstNodeView>(
              // ... in the same file as the current method
              (QNodeFilePath == method.filePath &&
              // ... within the textual scope of the current method's body.
              (QNodeRange.start.line >= start.line
                || (QNodeRange.start.line == start.line
                && QNodeRange.start.column >= start.column)) &&
              (QNodeRange.end.line <= end.line
                || (QNodeRange.end.line == end.line
                && QNodeRange.end.column <= end.column)))
            ))
          {
            // If this AST node is a reference to a field of the type...
            if (fieldHashes.find(node.entityHash) != fieldHashes.end())
            {
              // ... then mark it as used by this method.
              usedFields.insert(node.entityHash);
            }
          }
          
          ++methodCount;
          totalCohesion += usedFields.size();
        }
      }

      // Calculate and record metrics.
      const double dF = fieldCount;
      const double dM = methodCount;
      const double dC = totalCohesion;
      const bool trivial = fieldCount == 0 || methodCount == 0;
      const bool singular = methodCount == 1;
      
      // Standard lack of cohesion (range: [0,1])
      model::CppAstNodeMetrics lcm;
      lcm.astNodeId = type.astNodeId;
      lcm.type = model::CppAstNodeMetrics::Type::LACK_OF_COHESION;
      lcm.value = trivial ? 0.0 :
        (1.0 - dC / (dM * dF));
      _ctx.db->persist(lcm);

      // Henderson-Sellers variant (range: [0,2])
      model::CppAstNodeMetrics lcm_hs;
      lcm_hs.astNodeId = type.astNodeId;
      lcm_hs.type = model::CppAstNodeMetrics::Type::LACK_OF_COHESION_HS;
      lcm_hs.value = trivial ? 0.0 : singular ? NAN :
        ((dM - dC / dF) / (dM - 1.0));
      _ctx.db->persist(lcm_hs);
    }
  });
}

bool CppMetricsParser::parse()
{
  LOG(info) << "[cppmetricsparser] Computing function parameter count metric.";
  functionParameters();
  LOG(info) << "[cppmetricsparser] Computing McCabe metric for functions.";
  functionMcCabe();
  LOG(info) << "[cppmetricsparser] Computing Bumpy Road metric for functions.";
  functionBumpyRoad();
  LOG(info) << "[cppmetricsparser] Computing Lack of Cohesion metric for types.";
  lackOfCohesion();
  return true;
}

CppMetricsParser::~CppMetricsParser()
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
    boost::program_options::options_description description("C++ Metrics Plugin");

    return description;
  }

  std::shared_ptr<CppMetricsParser> make(ParserContext& ctx_)
  {
    return std::make_shared<CppMetricsParser>(ctx_);
  }
}
#pragma clang diagnostic pop

} // parser
} // cc
