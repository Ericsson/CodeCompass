#include <cppmetricsparser/cppmetricsparser.h>

#include <model/cppastnodemetrics.h>
#include <model/cppastnodemetrics-odb.hxx>
#include <model/cppfilemetrics.h>
#include <model/cppfilemetrics-odb.hxx>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>

#include <boost/filesystem.hpp>

#include <util/logutil.h>
#include <util/odbtransaction.h>

#include <memory>

namespace cc
{
namespace parser
{

CppMetricsParser::CppMetricsParser(ParserContext& ctx_): AbstractParser(ctx_)
{
  util::OdbTransaction {_ctx.db} ([&, this] {
    for (const model::CppFileMetrics& fm
      : _ctx.db->query<model::CppFileMetrics>())
    {
      _fileIdCache.insert(fm.file);
    }

    for (const model::CppAstNodeMetrics& anm
      : _ctx.db->query<model::CppAstNodeMetrics>())
    {
      auto node = _ctx.db->query_one<model::CppAstNode>(
        odb::query<model::CppAstNode>::id == anm.astNodeId);
      _astNodeIdCache.insert({anm.astNodeId, node->location.file->id});
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
      model::CppAstNodeMetrics funcParams;
      funcParams.astNodeId = paramCount.id;
      funcParams.type = model::CppAstNodeMetrics::Type::PARAMETER_COUNT;
      funcParams.value = paramCount.count;
      _ctx.db->persist(funcParams);
    }
  });
}

void CppMetricsParser::lackOfCohesion()
{
  util::OdbTransaction {_ctx.db} ([&, this]
  {
    // Simplify some type names for readability.
    typedef model::CppMemberType::Kind MemberKind;
    typedef model::CppAstNode::AstType AstType;
    typedef std::uint64_t HashType;

    typedef odb::query<model::CppMemberType> QMem;
    typedef odb::query<model::CppAstNode> QNode;
    
    // Calculate the cohesion metric for all types.
    for (const model::CppRecord& type
      : _ctx.db->query<model::CppRecord>())
    {
      std::unordered_set<HashType> fieldHashes;
      // Query all members...
      for (const model::CppMemberType& field
        : _ctx.db->query<model::CppMemberType>(
          // ... that are fields
          QMem::kind == MemberKind::Field &&
          // ... of the current type.
          QMem::typeHash == type.entityHash
        ))
      {
        // Record these fields for later use.
        fieldHashes.insert(field.memberAstNode->entityHash);
      }
      size_t fieldCount = fieldHashes.size();

      size_t methodCount = 0;
      size_t totalCohesion = 0;
      // Query all members...
      for (const model::CppMemberType& method
        : _ctx.db->query<model::CppMemberType>(
          // ... that are methods
          QMem::kind == MemberKind::Method &&
          // ... of the current type.
          QMem::typeHash == type.entityHash
        ))
      {
        // Do not consider methods with no explicit bodies.
        model::FileLoc methodLoc = method.memberAstNode->location;
        if (methodLoc.range.start < methodLoc.range.end)
        {
          std::unordered_set<HashType> usedFields;

          // Query all AST nodes...
          for (const model::CppAstNode& node
            : _ctx.db->query<model::CppAstNode>(
              // ... that use a variable for reading or writing
              (QNode::astType == AstType::Read ||
              QNode::astType == AstType::Write) &&
              // ... in the same file as the current method
              (QNode::location.file->path == methodLoc.file->path &&
              // ... within the textual scope of the current method's body.
              (QNode::location.range.start.line >= methodLoc.range.start.line
                || (QNode::location.range.start.line == methodLoc.range.start.line
                && QNode::location.range.start.column >= methodLoc.range.start.column)) &&
              (QNode::location.range.end.line <= methodLoc.range.end.line
                || (QNode::location.range.end.line == methodLoc.range.end.line
                && QNode::location.range.end.column <= methodLoc.range.end.column)))
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
      constexpr double scaling = 1e+4;
      
      // Standard lack of cohesion:
      model::CppAstNodeMetrics lcm;
      lcm.astNodeId = type.astNodeId;
      lcm.type = model::CppAstNodeMetrics::Type::LACK_OF_COHESION;
      lcm.value = static_cast<unsigned int>(scaling * 
        (1.0 - dC / (dM * dF)));// range: [0,1]
      _ctx.db->persist(lcm);

      // Henderson-Sellers variant:
      model::CppAstNodeMetrics lcm_hs;
      lcm_hs.astNodeId = type.astNodeId;
      lcm_hs.type = model::CppAstNodeMetrics::Type::LACK_OF_COHESION_HS;
      lcm_hs.value = static_cast<unsigned int>(scaling * 
        ((dM - dC / dF) / (dM - 1.0)));// range: [0,2]
      _ctx.db->persist(lcm_hs);
    }
  });
}

bool CppMetricsParser::parse()
{
  // Function parameter number metric.
  functionParameters();

  // Lack of cohesion within types.
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
