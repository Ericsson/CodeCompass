#include <cppmetricsparser/cppmetricsparser.h>

#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>
#include <model/cppcohesionmetrics.h>
#include <model/cppcohesionmetrics-odb.hxx>
#include <model/cppfilemetrics.h>
#include <model/cppfilemetrics-odb.hxx>
#include <model/cppinheritance.h>
#include <model/cppinheritance-odb.hxx>
#include <model/cpprecord.h>
#include <model/cpprecord-odb.hxx>
#include <model/cpptypedependencymetrics.h>
#include <model/cpptypedependencymetrics-odb.hxx>
#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>
#include <model/file.h>
#include <model/file-odb.hxx>

#include <boost/filesystem.hpp>

#include <util/filesystem.h>
#include <util/logutil.h>

#include <memory>

namespace cc
{
namespace parser
{

namespace fs = boost::filesystem;

CppMetricsParser::CppMetricsParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  _threadCount = _ctx.options["jobs"].as<int>();
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
  parallelCalcMetric<model::CppFunctionParamCountWithId>(
    "Function parameters",
    _threadCount * functionParamsPartitionMultiplier,// number of jobs; adjust for granularity
    getFilterPathsQuery<model::CppFunctionParamCountWithId>(),
    [&, this](const MetricsTasks<model::CppFunctionParamCountWithId>& tasks)
  {
    util::OdbTransaction {_ctx.db} ([&, this]
    {
      for (const model::CppFunctionParamCountWithId& param : tasks)
      {
        model::CppAstNodeMetrics funcParams;
        funcParams.astNodeId = param.id;
        funcParams.type = model::CppAstNodeMetrics::Type::PARAMETER_COUNT;
        funcParams.value = param.count;
        _ctx.db->persist(funcParams);
      }
    });
  });
}

void CppMetricsParser::functionMcCabe()
{
  parallelCalcMetric<model::CppFunctionMcCabe>(
    "Function-level McCabe",
    _threadCount * functionMcCabePartitionMultiplier,// number of jobs; adjust for granularity
    getFilterPathsQuery<model::CppFunctionMcCabe>(),
    [&, this](const MetricsTasks<model::CppFunctionMcCabe>& tasks)
  {
    util::OdbTransaction {_ctx.db} ([&, this]
    {
      for (const model::CppFunctionMcCabe& param : tasks)
      {
        model::CppAstNodeMetrics funcMcCabe;
        funcMcCabe.astNodeId = param.astNodeId;
        funcMcCabe.type = model::CppAstNodeMetrics::Type::MCCABE_FUNCTION;
        funcMcCabe.value = param.mccabe;
        _ctx.db->persist(funcMcCabe);
      }
    });
  });
}

void CppMetricsParser::functionBumpyRoad()
{
  // Calculate the bumpy road metric for all types on parallel threads.
  parallelCalcMetric<model::CppFunctionBumpyRoad>(
    "Bumpy road complexity",
    _threadCount * functionBumpyRoadPartitionMultiplier,// number of jobs; adjust for granularity
    getFilterPathsQuery<model::CppFunctionBumpyRoad>(),
    [&, this](const MetricsTasks<model::CppFunctionBumpyRoad>& tasks)
  {
    util::OdbTransaction {_ctx.db} ([&, this]
    {
      for (const model::CppFunctionBumpyRoad& function : tasks)
      {
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
  });
}

void CppMetricsParser::typeMcCabe()
{
  util::OdbTransaction {_ctx.db} ([&, this]
  {
    using MemberT    = model::CppMemberType;
    using AstNode    = model::CppAstNode;
    using Entity     = model::CppEntity;
    using AstNodeMet = model::CppAstNodeMetrics;

    std::map<model::CppAstNodeId, unsigned int> mcValues;

    // Process all class definitions
    for (const auto& type : _ctx.db->query<AstNode>(
      odb::query<AstNode>::symbolType == AstNode::SymbolType::Type &&
      odb::query<AstNode>::astType == AstNode::AstType::Definition))
    {
      // Skip if class is included from external library
      type.location.file.load();
      const auto typeFile = _ctx.db->query_one<model::File>(
        odb::query<model::File>::id == type.location.file->id);
      if (!typeFile || !cc::util::isRootedUnderAnyOf(_inputPaths, typeFile->path))
        continue;

      // Skip if its a template instantiation
      const auto typeEntity = _ctx.db->query_one<Entity>(
        odb::query<Entity>::astNodeId == type.id);
      if (typeEntity && typeEntity->tags.find(model::Tag::TemplateInstantiation)
                        != typeEntity->tags.cend())
        continue;

      mcValues[type.id] = 0;

      // Process its methods
      for (const auto& method : _ctx.db->query<MemberT>(
        odb::query<MemberT>::typeHash == type.entityHash &&
        odb::query<MemberT>::kind == MemberT::Kind::Method))
      {
        // Lookup AST node of method
        method.memberAstNode.load();
        const auto methodAstNode = _ctx.db->query_one<AstNode>(
          odb::query<AstNode>::id == method.memberAstNode->id);
        if (!methodAstNode)
          continue;

        // Lookup its definition (different AST node if not defined in class body)
        auto methodDefs = _ctx.db->query<AstNode>(
          odb::query<AstNode>::entityHash == methodAstNode->entityHash &&
          odb::query<AstNode>::symbolType == AstNode::SymbolType::Function &&
          odb::query<AstNode>::astType == AstNode::AstType::Definition);
        if (methodDefs.empty())
          continue;
        const auto methodDef = *methodDefs.begin();
        // Note: we cannot use query_one, because a project might have multiple
        // functions with the same entityHash compiled to different binaries
        // So we take the first result, which introduces a small level of
        // potential inaccuracy
        // This could be optimized in the future if linkage information about
        // translation units got added to the database

        // Skip implicitly defined methods (constructors, operator=, etc.)
        const auto entity = _ctx.db->query_one<Entity>(
          odb::query<Entity>::astNodeId == methodDef.id);
        if (entity && entity->tags.find(model::Tag::Implicit) != entity->tags.cend())
          continue;

        // Lookup metrics of this definition
        const auto funcMetrics = _ctx.db->query_one<AstNodeMet>(
          odb::query<AstNodeMet>::astNodeId == methodDef.id &&
          odb::query<AstNodeMet>::type == model::CppAstNodeMetrics::Type::MCCABE_FUNCTION);
        if (funcMetrics)
        {
          // Increase class mccabe by the method's
          mcValues[type.id] += funcMetrics->value;
        }
      }
    }

    for (const auto& mcValue : mcValues)
    {
      model::CppAstNodeMetrics typeMcMetric;
      typeMcMetric.astNodeId = mcValue.first;
      typeMcMetric.type = model::CppAstNodeMetrics::Type::MCCABE_TYPE;
      typeMcMetric.value = mcValue.second;
      _ctx.db->persist(typeMcMetric);
    }
  });
}

void CppMetricsParser::lackOfCohesion()
{
  // Calculate the cohesion metric for all types on parallel threads.
  parallelCalcMetric<model::CohesionCppRecordView>(
    "Lack of cohesion",
    _threadCount * lackOfCohesionPartitionMultiplier, // number of jobs; adjust for granularity
    getFilterPathsQuery<model::CohesionCppRecordView>(),
    [&, this](const MetricsTasks<model::CohesionCppRecordView>& tasks)
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

      for (const model::CohesionCppRecordView& type : tasks)
      {
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

            // Query AST nodes that use a variable for reading or writing...
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
  });
}

void CppMetricsParser::efferentTypeLevel()
{
  parallelCalcMetric<model::CohesionCppRecordView>(
    "Efferent coupling of types",
    _threadCount * efferentCouplingTypesPartitionMultiplier,// number of jobs; adjust for granularity
    getFilterPathsQuery<model::CohesionCppRecordView>(),
    [&, this](const MetricsTasks<model::CohesionCppRecordView>& tasks)
    {
      util::OdbTransaction{_ctx.db}([&, this]
      {
        typedef odb::query<cc::model::CppMemberType> MemTypeQuery;
        typedef odb::query<cc::model::CppInheritanceCount> InheritanceQuery;
        typedef odb::query<cc::model::CppFunctionParamTypeView> ParamQuery;
        typedef odb::query<cc::model::CppFunctionLocalTypeView> LocalQuery;
        typedef odb::query<cc::model::CppFunction> FuncQuery;

        std::set<std::uint64_t> dependentTypes;
        for (const model::CohesionCppRecordView& type : tasks)
        {
          dependentTypes.clear();

          // Count parent types
          auto inheritanceView = _ctx.db->query<model::CppInheritance>(
            InheritanceQuery::derived == type.entityHash);
          auto inheritanceCount = _ctx.db->query_value<model::CppInheritanceCount>(
            InheritanceQuery::derived == type.entityHash);

          // Count unique attribute types
          // and unique types in function parameters and local variables
          for (const model::CppMemberType& mem: _ctx.db->query<model::CppMemberType>(
            MemTypeQuery::typeHash == type.entityHash))
          {
            auto funcAstNodeId = mem.memberAstNode.load()->id;

            if (mem.kind == cc::model::CppMemberType::Field)
            {
              dependentTypes.insert(mem.memberTypeHash);
            }
            else
            {
              for (const auto& param: _ctx.db->query<model::CppFunctionParamTypeView>(
                FuncQuery::astNodeId == funcAstNodeId))
              {
                dependentTypes.insert(param.paramTypeHash);
              }

              for (const auto& local: _ctx.db->query<model::CppFunctionLocalTypeView>(
                FuncQuery::astNodeId == funcAstNodeId))
              {
                dependentTypes.insert(local.paramTypeHash);
              }
            }
          }

          model::CppAstNodeMetrics metric;
          metric.astNodeId = type.astNodeId;
          metric.type = model::CppAstNodeMetrics::Type::EFFERENT_TYPE;
          metric.value = inheritanceCount.count + dependentTypes.size();
          _ctx.db->persist(metric);

          auto typeRelationInserter = [this](const std::uint64_t& entityHash, const std::uint64_t& dependencyHash)
          {
            model::CppTypeDependencyMetrics relation;
            relation.entityHash = entityHash;
            relation.dependencyHash = dependencyHash;
            _ctx.db->persist(relation);
          };

          // Insert type dependency relations
          for (const std::uint64_t& d : dependentTypes) {
            typeRelationInserter(type.entityHash, d);
          }

          // Insert inheritance relations
          for (const model::CppInheritance& d : inheritanceView) {
            typeRelationInserter(type.entityHash, d.base);
          }
        }
      });
  });
}

void CppMetricsParser::afferentTypeLevel()
{
  parallelCalcMetric<model::CohesionCppRecordView>(
    "Afferent coupling of types",
    _threadCount * afferentCouplingTypesPartitionMultiplier,// number of jobs; adjust for granularity
    getFilterPathsQuery<model::CohesionCppRecordView>(),
    [&, this](const MetricsTasks<model::CohesionCppRecordView>& tasks)
    {
      util::OdbTransaction{_ctx.db}([&, this]
      {
        typedef odb::query<cc::model::CppAstNode> AstQuery;
        typedef odb::query<cc::model::CppInheritance> InheritanceQuery;
        typedef odb::query<cc::model::CppMemberType> MemTypeQuery;
        typedef odb::result<cc::model::CppAstNode> AstResult;
        typedef odb::result<cc::model::CppMemberTypeAstView> MemTypeAstResult;

        std::set<std::uint64_t> dependentTypes;
        for (const model::CohesionCppRecordView& type : tasks)
        {
          dependentTypes.clear();

          // Find derived types
          for (const model::CppInheritance& inheritance : _ctx.db->query<model::CppInheritance>(
            InheritanceQuery::base == type.entityHash))
          {
            dependentTypes.insert(inheritance.derived);
          }

          // Find usages of the type
          for (const model::CppAstNode& usage : _ctx.db->query<model::CppAstNode>(
              AstQuery::entityHash == type.entityHash &&
              AstQuery::location.range.end.line != model::Position::npos))
          {
            // Check if usage is in class member function or attribute
            MemTypeAstResult memberNode = _ctx.db->query<model::CppMemberTypeAstView>(
                AstQuery::symbolType.in(model::CppAstNode::SymbolType::Function, model::CppAstNode::SymbolType::Variable) &&
                AstQuery::astType.in(model::CppAstNode::AstType::Definition, model::CppAstNode::AstType::Declaration) &&
                AstQuery::location.file == usage.location.file.object_id() &&
                AstQuery::location.range.start.line <= usage.location.range.start.line &&
                AstQuery::location.range.end.line >= usage.location.range.end.line &&
                MemTypeQuery::typeHash != usage.entityHash);

            if (!memberNode.empty())
            {
              dependentTypes.insert(memberNode.begin()->typeHash);
            } else {
              // The usage can be in a member function defined outside of the class definition
              // E.g. void ClassName::foo() { A a; }
              //                              ^ usage here

              // Find parent function
              AstResult parentFunction = _ctx.db->query<model::CppAstNode>(
                AstQuery::symbolType == model::CppAstNode::SymbolType::Function &&
                AstQuery::astType == model::CppAstNode::AstType::Definition &&
                AstQuery::location.file == usage.location.file.object_id() &&
                AstQuery::location.range.start.line <= usage.location.range.start.line &&
                AstQuery::location.range.end.line >= usage.location.range.end.line);

              if (!parentFunction.empty())
              {
                // Find if the function is a member function of a class
                MemTypeAstResult memberFunction = _ctx.db->query<model::CppMemberTypeAstView>(
                  AstQuery::entityHash == parentFunction.begin()->entityHash &&
                  MemTypeQuery::typeHash != usage.entityHash);

                if (!memberFunction.empty())
                {
                  dependentTypes.insert(memberFunction.begin()->typeHash);
                }
              }
            }
          }

          model::CppAstNodeMetrics metric;
          metric.astNodeId = type.astNodeId;
          metric.type = model::CppAstNodeMetrics::Type::AFFERENT_TYPE;
          metric.value = dependentTypes.size();
          _ctx.db->persist(metric);
        }
      });
  });
}

odb::query<model::File> CppMetricsParser::getModulePathsQuery()
{
  if (_ctx.moduleDirectories.empty()) {
    // No module directories specified, compute for all directories
    return odb::query<model::File>::type == cc::model::File::DIRECTORY_TYPE && getFilterPathsQuery<model::File>();
  } else {
    // Compute for module directories
    return odb::query<model::File>::path.in_range(_ctx.moduleDirectories.begin(), _ctx.moduleDirectories.end());
  }
}

void CppMetricsParser::efferentModuleLevel()
{
  parallelCalcMetric<model::File>(
    "Efferent coupling at module level",
    _threadCount * efferentCouplingModulesPartitionMultiplier,// number of jobs; adjust for granularity
    getModulePathsQuery(),
    [&, this](const MetricsTasks<model::File>& tasks)
    {
      util::OdbTransaction{_ctx.db}([&, this]
      {
        typedef odb::query<model::CppTypeDependencyMetrics_Distinct_D_Count> TypeDependencyQuery;
        typedef model::CppTypeDependencyMetrics_Distinct_D_Count TypeDependencyResult;

        for (const model::File& file : tasks)
        {
          TypeDependencyResult types = _ctx.db->query_value<model::CppTypeDependencyMetrics_Distinct_D_Count>(
            TypeDependencyQuery::EntityFile::path.like(file.path + '%') &&
            !TypeDependencyQuery::DependencyFile::path.like(file.path + '%'));

          model::CppFileMetrics metric;
          metric.file = file.id;
          metric.type = model::CppFileMetrics::Type::EFFERENT_MODULE;
          metric.value = types.count;
          _ctx.db->persist(metric);
        }
      });
  });
}

void CppMetricsParser::afferentModuleLevel()
{
  parallelCalcMetric<model::File>(
    "Afferent coupling at module level",
    _threadCount * afferentCouplingModulesPartitionMultiplier,// number of jobs; adjust for granularity
    getModulePathsQuery(),
    [&, this](const MetricsTasks<model::File>& tasks)
    {
      util::OdbTransaction{_ctx.db}([&, this]
      {
        typedef odb::query<model::CppTypeDependencyMetrics_Distinct_E_Count> TypeDependencyQuery;
        typedef model::CppTypeDependencyMetrics_Distinct_E_Count TypeDependencyResult;

        for (const model::File& file : tasks)
        {
          TypeDependencyResult types = _ctx.db->query_value<model::CppTypeDependencyMetrics_Distinct_E_Count>(
            !TypeDependencyQuery::EntityFile::path.like(file.path + '%') &&
            TypeDependencyQuery::DependencyFile::path.like(file.path + '%'));

          model::CppFileMetrics metric;
          metric.file = file.id;
          metric.type = model::CppFileMetrics::Type::AFFERENT_MODULE;
          metric.value = types.count;
          _ctx.db->persist(metric);
        }
      });
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
  LOG(info) << "[cppmetricsparser] Computing McCabe metric for types.";
  typeMcCabe();
  LOG(info) << "[cppmetricsparser] Computing Lack of Cohesion metric for types.";
  lackOfCohesion();
  LOG(info) << "[cppmetricsparser] Computing efferent coupling metric for types.";
  efferentTypeLevel();
  LOG(info) << "[cppmetricsparser] Computing afferent coupling metric for types.";
  afferentTypeLevel();
  LOG(info) << "[cppmetricsparser] Computing efferent coupling metric at module level.";
  efferentModuleLevel(); // This metric needs to be calculated after efferentTypeLevel
  LOG(info) << "[cppmetricsparser] Computing afferent coupling metric at module level.";
  afferentModuleLevel(); // This metric needs to be calculated after afferentTypeLevel
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
