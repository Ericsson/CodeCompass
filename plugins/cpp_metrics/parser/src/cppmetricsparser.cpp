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

<<<<<<< HEAD
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

=======
>>>>>>> 61a1f5fe2912f2abe1c177573b0e7c828b5577af
void CppMetricsParser::relationalCohesion()
{
  util::OdbTransaction {_ctx.db} ([&, this]
  {

    std::unordered_set<std::string> filepaths(_inputPaths.begin(),_inputPaths.end());
    std::unordered_map<std::uint64_t,std::string> typeDefinitionPaths;
    std::unordered_set<std::uint64_t> typesFound;



    //TODO: implement module handling logic

    std::unordered_map<std::string,std::uint64_t> moduleIds;

    for (auto &&path : filepaths)
    {
      for(const model::RelationalCohesionFileView& file
      : _ctx.db->query<model::RelationalCohesionFileView>(
        odb::query<model::RelationalCohesionFileView>::query_columns::path.equal(path)
      ))
      {
        moduleIds[path] = file.fileId;
      }
    }
    

    for (auto &&path : filepaths)
    {

      typesFound.clear();
      typeDefinitionPaths.clear();
      //find the types defined in the module
      for (const model::RelationalCohesionRecordView& record
      : _ctx.db->query<model::RelationalCohesionRecordView>(
        odb::query<model::RelationalCohesionRecordView>::query_columns::File::path.like(path + "%")
      ))
      {
        typesFound.insert(record.typeHash); //save types defined inside the module
        typeDefinitionPaths.insert(std::make_pair(record.typeHash,record.filePath)); //save where the type is defined to avoid self relation
      }
      
      std::unordered_map<std::string,std::unordered_set<std::uint64_t>> relationsFoundInFile; //store the type relations already found for each file
      int relationsInModule = 0;

      //check function return types
      for (const model::RelationalCohesionFunctionView& function
      : _ctx.db->query<model::RelationalCohesionFunctionView>(
        odb::query<model::RelationalCohesionFunctionView>::query_columns::File::path.like(path + "%")
      ))
      {
        if (
          typesFound.find(function.returnType) != typesFound.end() &&
          //check if return type is defined within the current module
          typeDefinitionPaths[function.returnType] != function.filePath &&
          //check for self relation
          (relationsFoundInFile.find(function.filePath) == relationsFoundInFile.end() ||
          //check if any reletaions were found in the current file
          relationsFoundInFile[function.filePath].find(function.returnType) == relationsFoundInFile[function.filePath].end())
          //check if this relation is already found
        )
        {
          ++relationsInModule;
          auto it = relationsFoundInFile.find(function.filePath);
          if (it != relationsFoundInFile.end())
          {
            it->second.insert(function.returnType);
          }
          else
          {
            relationsFoundInFile.insert(std::make_pair(function.filePath,std::unordered_set<std::uint64_t>{function.returnType}));
          }
        }
        
      }

      //check function parameters
      for (const model::RelationalCohessionFunctionParameterView& parameter
      : _ctx.db->query<model::RelationalCohessionFunctionParameterView>(
        odb::query<model::RelationalCohessionFunctionParameterView>::query_columns::File::path.like(path + "%")
      ))
      {
        if (
          typesFound.find(parameter.typeHash) != typesFound.end() &&
          //check if return type is defined within the current module
          typeDefinitionPaths[parameter.typeHash] != parameter.filePath &&
          //check for self relation
          (relationsFoundInFile.find(parameter.filePath) == relationsFoundInFile.end() ||
          //check if any reletaions were found in the current file
          relationsFoundInFile[parameter.filePath].find(parameter.typeHash) == relationsFoundInFile[parameter.filePath].end())
          //check if this relation is already found
        )
        {
          ++relationsInModule;
          auto it = relationsFoundInFile.find(parameter.filePath);
          if (it != relationsFoundInFile.end())
          {
            it->second.insert(parameter.typeHash);
          }
          else
          {
            relationsFoundInFile.insert(std::make_pair(parameter.filePath,std::unordered_set<std::uint64_t>{parameter.typeHash}));
          }
        }
        
      }

      //check function locals
      for (const model::RelationalCohessionFunctionLocalView& local
      : _ctx.db->query<model::RelationalCohessionFunctionLocalView>(
        odb::query<model::RelationalCohessionFunctionLocalView>::query_columns::File::path.like(path + "%")
      ))
      {
        if (
          typesFound.find(local.typeHash) != typesFound.end() &&
          //check if return type is defined within the current module
          typeDefinitionPaths[local.typeHash] != local.filePath &&
          //check for self relation
          (relationsFoundInFile.find(local.filePath) == relationsFoundInFile.end() ||
          //check if any reletaions were found in the current file
          relationsFoundInFile[local.filePath].find(local.typeHash) == relationsFoundInFile[local.filePath].end())
          //check if this relation is already found
        )
        {
          ++relationsInModule;
          auto it = relationsFoundInFile.find(local.filePath);
          if (it != relationsFoundInFile.end())
          {
            it->second.insert(local.typeHash);
          }
          else
          {
            relationsFoundInFile.insert(std::make_pair(local.filePath,std::unordered_set<std::uint64_t>{local.typeHash}));
          }
        }
        
      }

      //check variables
      for (const model::RelationalCohesionVariableView& variable
      : _ctx.db->query<model::RelationalCohesionVariableView>(
        odb::query<model::RelationalCohesionVariableView>::query_columns::File::path.like(path + "%")
      ))
      {
        if (
          typesFound.find(variable.typeHash) != typesFound.end() &&
          //check if return type is defined within the current module
          typeDefinitionPaths[variable.typeHash] != variable.filePath &&
          //check for self relation
          (relationsFoundInFile.find(variable.filePath) == relationsFoundInFile.end() ||
          //check if any reletaions were found in the current file
          relationsFoundInFile[variable.filePath].find(variable.typeHash) == relationsFoundInFile[variable.filePath].end())
          //check if this relation is already found
        )
        {
          ++relationsInModule;
          auto it = relationsFoundInFile.find(variable.filePath);
          if (it != relationsFoundInFile.end())
          {
            it->second.insert(variable.typeHash);
          }
          else
          {
            relationsFoundInFile.insert(std::make_pair(variable.filePath,std::unordered_set<std::uint64_t>{variable.typeHash}));
          }
        }
        
      }

      //calculate relational cohesion for module
      //formula: H = (R + 1)/ N
      //where H is the relational cohesion value,
      //R is the number of relationships internal to the module,
      //N is the number of types in the module

      uint numberOfTypesInModule = typesFound.size(); //N

      double relationalCohesion = 
        (static_cast<double>(relationsInModule) + 1.0) / static_cast<double>(numberOfTypesInModule);

      model::CppFileMetrics relationalCohesionMetrics;
      relationalCohesionMetrics.file = moduleIds[path];
      relationalCohesionMetrics.value = relationalCohesion;
      relationalCohesionMetrics.type = model::CppFileMetrics::Type::RELATIONAL_COHESION;
      _ctx.db->persist(relationalCohesionMetrics);

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
  relationalCohesion();
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
