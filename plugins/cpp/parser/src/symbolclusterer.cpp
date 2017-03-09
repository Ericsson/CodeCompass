#include <algorithm>
#include <sstream>
#include <unordered_set>
#include <utility>

#include <boost/algorithm/string/replace.hpp>
#include <boost/lambda/lambda.hpp>

#include <util/odbtransaction.h>

#include <model/file-odb.hxx>
#include <model/cppastnode.h>
#include <model/cppclusterinfo-odb.hxx>
#include <model/statistics.h>
#include <model/statistics-odb.hxx>

#include "symbolclusterer.h"

namespace
{

/**
 * @brief Convert the given ID to a string for building verbatim query strings.
 *
 * This method is needed because the code could contain unsigned IDs which
 * are translated as signed integers in the database. The conversion is done
 * automatically by ODB, but not in case of verbatim query strings, which are
 * heavily used in the implementation of this module. (Thus, without a manual
 * conversion, result sets would be almost always empty.)
 *
 * @tparam Object The type of the database object, which must be an
 *                ODB database object with an 'id' primary key defined in it.
 * @tparam ID     The type of the ID that is passed as an argument.
 * @param id    The database object's actual ID to convert.
 * @return      The id converted to an std::string.
 */
template <typename Object, typename ID>
std::string convertIdToVerbatim(const ID& id);

} // (anonymous)


namespace cc
{
namespace parser
{

/** SymbolClusterer **/
SymbolClusterer::SymbolClusterer(ParserContext& ctx)
  : _ctx(ctx)
  , _calculatorPool(nullptr)
{
}

SymbolClusterer::cluster_collection
SymbolClusterer::retrieveInitialClusters(size_t threadCount)
{
  // For optimisation reasons, this method builds a huge SQL query to
  // put as much calculation load as possible on the database.

  /** Step one **/
  // Get the queries which mark the enhancable mangled name equivalence groups
  // for us.
  SymbolClusterer::marker_map mnMarkers = getMangledNameMarkers();

  /** Step two **/
  // Build the SQL query
  const std::string queryString = buildViewQuery(mnMarkers);

  /** Step three **/
  // We execute the query and retrieve the results.
  // This code does not commit the transaction as it only executes SELECTs.
  odb::transaction trans(_ctx.db->begin());

  odb::result<model::FileToClusterView> res(
    _ctx.db->query<model::FileToClusterView>(queryString));

  // The header propagation part is executed multi-thread.
  typedef std::pair<model::FileId, SymbolClusterer::actions&> HeaderPropagation;
  SymbolClusterer::cluster_collection map;
  size_t i = 0;

  std::unique_ptr<util::JobQueueThreadPool<HeaderPropagation>>
    headerPropagator;
  if (threadCount == 1)
  {
    // The single-threaded code can NOT create a transaction as it would
    // result in an error. We branch here instead of inside the lambda so that
    // overhead is minimal.
    auto headerPropagation = [this, &i](HeaderPropagation& job)
    {
      this->propagateHeaderIntoInitialCluster(job.first, job.second);
      i += job.second.size();
    };

    headerPropagator = util::make_thread_pool<HeaderPropagation>(
      threadCount, headerPropagation);
  }
  else
  {
    auto headerPropagation = [this, &i](HeaderPropagation &job)
    {
      odb::transaction t(this->_ctx.db->begin());
      this->propagateHeaderIntoInitialCluster(job.first, job.second);
      i += job.second.size();
    };

    headerPropagator = util::make_thread_pool<HeaderPropagation>(
      threadCount, headerPropagation);
  }

  for (auto row : res)
  {
    if (!row.build_action.null() && row.header_inclusion.null())
    {
      // A source file has the initial cluster for it already calculated,
      // we only need to store this information for further expansion.
      map[row.file].push_back(*row.build_action);
      ++i;
    }
    else if (row.build_action.null() && !row.header_inclusion.null())
    {
      // Ensure that the vector for the header exists.
      map[row.file].clear();

      // The header-->build_action calculation is ran asynchronously.
      headerPropagator->enqueue(std::make_pair(row.file,
                                               std::ref(map.at(row.file))));
    }
    else
    {
      model::File f;
      _ctx.db->load<model::File>(row.file, f);
      LOG(warning) << "'" << f.path << "' is marked file containing "
                      "\"problematic\" symbols never built or #included! "
                      "Cannot clusterise this file!";
    }
  }

  // Wait for the header propagations to finish before returning the full map.
  headerPropagator->wait();

  LOG(debug) << "Clustering: " << map.size() << " files initially problematic "
                "in " << i << " combinations";

  return map;
}

void SymbolClusterer::performClusterisation(cluster_collection& clusterMap,
                                            size_t threadCount)
{
  if (_calculatorPool)
    return;

  // Set up the pool which will execute the calculation method for each action.
  auto calculator = [this](SymbolClusterer::cluster_vref pair)
  {
    odb::transaction t(this->_ctx.db->begin());
    this->calculateBuildActionLeaves(pair.get().second);
  };

  _calculatorPool = util::make_thread_pool<SymbolClusterer::cluster_vref>(
    threadCount, calculator);

  for (auto& it : clusterMap)
    _calculatorPool->enqueue(std::ref(it));
}

void SymbolClusterer::waitForClusters()
{
  if (!_calculatorPool)
    return;

  _calculatorPool->wait();
}

SymbolClusterer::marker_map SymbolClusterer::getMangledNameMarkers()
{
  // Register the list of mangled name markers.
  // These markers will point us in the direction of mangled names on which
  // accuracy could be enhanced.
  SymbolClusterer::marker_map mnMarkers;

  // Check for function name collisions.
  {
    // A mangled name is UNIQUE in the entire project, and thus the accuracy
    // is not enhanceable:
    // * if there is only one definition
    // * |OR| no definition and only one declaration
    // * (|OR| no definition/declaration at all - this is an edge case)

    auto it = mnMarkers.emplace("function",
      SymbolClusterer::MangledNameMarker(R"end(
SELECT
  "ALL"."mangledName" AS "mangledName",
  "ALL"."count" AS "count_all",
  "Decls"."count" AS "count_decl",
  "Defs"."count" AS "def_count"
FROM
(
  SELECT COUNT(*) AS "count", "mangledName"
  FROM "CppAstNode"
    WHERE "symbolType" = $SYMTYPE_FUNC$
  GROUP BY "mangledName"
) "ALL"
FULL OUTER JOIN
(
  SELECT COUNT(*) AS "count", "mangledName"
  FROM "CppAstNode"
    WHERE "symbolType" = $SYMTYPE_FUNC$
      AND "astType" = $ASTTYPE_DECL$
  GROUP BY "mangledName"
) "Decls" ON "ALL"."mangledName" = "Decls"."mangledName"
FULL OUTER JOIN
(
  SELECT COUNT(*) AS "count", "mangledName"
  FROM "CppAstNode"
    WHERE "symbolType" = $SYMTYPE_FUNC$
      AND "astType" = $ASTTYPE_DEF$
  GROUP BY "mangledName"
) "Defs" ON "ALL"."mangledName" = "Defs"."mangledName"
WHERE NOT
(
  "Defs"."count" = 1
  OR ("Defs"."count" IS NULL AND ("Decls"."count" = 1 OR "Decls"."count" IS NULL) )
)
ORDER BY "mangledName" ASC)end")).first;

    // The number representing the enum can change between versions, so we
    // register the replace rule dynamically.
    it->second.addReplaceRule("$SYMTYPE_FUNC$", std::to_string(
      static_cast<size_t>(model::CppAstNode::SymbolType::Function)));
    it->second.addReplaceRule("$ASTTYPE_DECL$", std::to_string(
      static_cast<size_t>(model::CppAstNode::AstType::Declaration)));
    it->second.addReplaceRule("$ASTTYPE_DEF$", std::to_string(
      static_cast<size_t>(model::CppAstNode::AstType::Definition)));
  }

  // Check for type collisions.
  {
    // A type is considered UNIQUE in the entire project if:
    // * it is defined only once
    // * OR if it is not defined at all (in that case it IS NULL)
    //
    // If it is defined multiple times (agnostic to declaration count)
    // jumping to the definition is problematic.

    auto it = mnMarkers.emplace(std::make_pair("type",
      SymbolClusterer::MangledNameMarker(std::string(R"end(SELECT
  "mangledName",
  COUNT(*) AS "count"
FROM "CppAstNode"
  WHERE "symbolType" = $SYMTYPE_TYPE$
    AND "astType" = $ASTTYPE_DEF$
GROUP BY "mangledName"
HAVING COUNT(*) > 1
ORDER BY "mangledName" ASC)end")))).first;

    it->second.addReplaceRule("$SYMTYPE_TYPE$", std::to_string(
      static_cast<size_t>(model::CppAstNode::SymbolType::Type)));
    it->second.addReplaceRule("$ASTTYPE_DEF$", std::to_string(
      static_cast<size_t>(model::CppAstNode::AstType::Definition)));
  }

  return mnMarkers;
}

std::string SymbolClusterer::buildViewQuery(
  const SymbolClusterer::marker_map& markers)
{
  std::ostringstream queryString;
  bool isFirst = true;

  queryString << "WITH ";
  for (auto marker : markers)
  {
    if (!isFirst)
      queryString << ", ";

    queryString << "\"mnMarker_" << marker.first << "\" AS ("
                << marker.second() << ")\n";
    isFirst = false;
  }

  // The symbols pointed by the mangled names could be present in any number of
  // files. These files are "concerned", i.e. they contain problematic
  // symbols, e.g. function calls which are ambiguous if we only consider
  // resolution based on the mangled name.
  queryString << R"end(, "filesConcerned" AS (
  SELECT DISTINCT "location_file" AS "id"
  FROM "CppAstNode"
  WHERE "CppAstNode"."mangledName" IN (
)end";

  // Get the list of mangled names from the marker results into this query.
  isFirst = true;

  for (auto marker : markers)
  {
    if (!isFirst)
      queryString << "\n    UNION ALL\n";

    queryString << "    SELECT \"mangledName\" FROM \"mnMarker_"
                << marker.first << "\"";
    isFirst = false;
  }

  // The files in "filesConcerned" are then mapped to the initial cluster set:
  // The build actions to which the files directly belong to.
  queryString << R"end(
  )
    AND "CppAstNode"."location_file" IS NOT NULL
)
SELECT DISTINCT "source_id", "action_id", "header_inclusion" FROM
()end";

  /** A symbol might be found in a HEADER. **/
  // Header files are always included (by the preprocessor) and we record that
  // information. Unfortunately, headers are not mapped to a direct build action,
  // and thus cluster calculation needs to take an extra step.
  queryString << R"end(
  SELECT DISTINCT "filesConcerned"."id" AS "source_id",
                  CAST(NULL AS BIGINT) AS "action_id",
                  CAST(1 AS BIGINT) AS "header_inclusion"
    FROM "filesConcerned"
	    INNER JOIN "CppHeaderInclusion" ON "filesConcerned"."id" = "CppHeaderInclusion"."included")end";

  /** Or a symbol might be found in a SOURCE file **/
  // Source files can directly be mapped to initial clusters.
  queryString << R"end(
UNION
  SELECT DISTINCT "filesConcerned"."id" AS "source_id",
                  "BuildSource"."action" AS "action_id",
                  CAST(NULL AS BIGINT) AS "header_inclusion"
    FROM "filesConcerned"
	    INNER JOIN "BuildSource" ON "filesConcerned"."id" = "BuildSource"."file"
) "tbl"
ORDER BY "source_id" ASC;
)end";

  return queryString.str();
}

template <typename GroupKey,
          typename View,
          typename Result,
          typename QueryBuilder,
          typename ExpandPredicate>
void SymbolClusterer::calculateReachablePropagation(
  Result& result,
  QueryBuilder queryBuilder,
  ExpandPredicate shouldExpand = boost::lambda::constant(true)) const
{
  Result leafCollector;

  bool couldExpand = true;
  while (couldExpand)
  {
    couldExpand = false;

    // Every time there was a new node in the reachable propagation,
    // we have to try to propagate from it again, which we collect here.
    std::map<GroupKey, Result> expand;

    std::string query = queryBuilder(result.cbegin(), result.cend());

    for (auto res : result)
      // Clear the value collection from expand to ensure an empty collection
      // exists for each key.
      expand[res].clear();

    // Because we are expanding the current result set and the previous
    // loop execution's query has been selected for this expansion (so IDs are
    // not lost), empty the result, which, at this point, acts as a
    // transient collection.
    result.clear();

    // Query the database to get the next set of nodes that could be expanded.
    odb::result<View> dbResult(_ctx.db->query<View>(query));

    // Iterate the database results
    for (View row : dbResult)
      // If, for the queried keys exist a value (which could not be the case!),
      // append it to the expansion map.
      if (*row.value && shouldExpand(row.key, *row.value))
        expand.at(row.key).insert(expand.at(row.key).end(), *row.value);

    // Handle chances of further expansion
    for (auto it : expand)
    {
      if (it.second.empty())
        // The current key represents a leaf in the chain.
        leafCollector.insert(leafCollector.end(), it.first);
      else
      {
        // The key is not a leaf, thus further expansion can happen.
        // Add the possible expandable keys to the transient collection.
        result.insert(result.end(), it.second.begin(), it.second.end());
        couldExpand = true;
      }
    }
  }

  // If expansion is over, we need to clean up the leaf collection
  // (DISTINCT it), and the output is generated in the "result" variable.
  std::sort(leafCollector.begin(), leafCollector.end());
  leafCollector.erase(
    std::unique(
      leafCollector.begin(), leafCollector.end()
    ),
    leafCollector.end()
  );

  result = std::move(leafCollector);
}

void SymbolClusterer::calculateBuildActionLeaves(
  SymbolClusterer::actions& actions)
{
  // actions_ contains the initial build actions where the given file_ is
  // seen as a source code.

  // Calculate the leaves using the eliminated transitive closure relation.
  // Symbols are propagated into subsequent build actions if the BuildTarget
  // of a BuildAction is the BuildSource of another BuildAction.
  this->calculateReachablePropagation<decltype(model::BuildAction::id),
                                      model::BuildActionExpandView>(
    actions,
    [](auto begin, auto end) {
      std::ostringstream query;

      query << R"end(SELECT DISTINCT "targetFilesForActions"."input_action",
                "BuildSource"."action" AS "expand_action"
FROM "BuildSource"
RIGHT OUTER JOIN (
  SELECT "file" AS "candidate_target", "action" AS "input_action"
  FROM "BuildTarget"
  WHERE "action" IN ()end";
    bool isFirst = true;
    for (; begin != end; ++begin)
    {
      if (!isFirst)
        query << ", ";
      query << convertIdToVerbatim<model::BuildAction>(*begin);
      isFirst = false;
    }

    query << R"end()
) "targetFilesForActions" ON "targetFilesForActions"."candidate_target" = "BuildSource"."file"
ORDER BY "input_action" ASC;)end";

    return query.str();
  });
}

void SymbolClusterer::propagateHeaderIntoInitialCluster(
  const model::FileId &file, SymbolClusterer::actions& actions)
{
  // The expansion starts from the file parameter.
  std::vector<model::FileId> files;
  files.push_back(file);

  // For headers, we first need to calculate the propagation of the
  // "is-included-into" relation's chain.

  // Header files can cyclically include each other which, without
  // countermeasures, results in an infinite loop when the chain is calculated.
  // (CodeCompass doesn't understand the concept of "include guards" as it only
  // parses the header as a single file containing code.)

  // An unordered_set is best for deduplication because IDs are easily
  // hashable and unordered_set provides O(1) lookup times.
  std::unordered_set<model::FileId> visitedHeaders;

  this->calculateReachablePropagation<model::FileId,
                                      model::HeaderInclusionExpandView>(
    files,
    [](auto begin, auto end) {
      std::ostringstream query;

      query << R"end(SELECT DISTINCT "includedFileIds"."id" AS "included",
                "CppHeaderInclusion"."includer"
FROM "CppHeaderInclusion"
RIGHT OUTER JOIN (
)end";
      bool isFirst = true;
      for (; begin != end; ++begin)
      {
        if (!isFirst)
          query << "UNION\n";

        // We print every ID as a "SELECT FROM DUAL" constant row, because
        // the ends of the propagation chain must be represented as an
        // (ID, NULL) record.
        query << "SELECT " << convertIdToVerbatim<model::File>(*begin)
              << " AS \"id\" ";
        isFirst = false;
      }

      query << R"end(
) "includedFileIds" ON "includedFileIds"."id" = "CppHeaderInclusion"."included"
ORDER BY "included" ASC;)end";

      return query.str();
    },
    [&visitedHeaders](const model::FileId /*included*/,
                      const model::FileId includer)
    {
      // If we have already seen the includer file, we should not continue
      // the chain as it would result in an infinite expansion.
      return visitedHeaders.insert(includer).second;
    });

  // files now contains the list of files the input file is included into.
  std::ostringstream query;
  {
    bool isFirst = true;
    query << R"end(SELECT DISTINCT "action"
FROM "BuildSource"
WHERE "file" IN ()end";

    for (auto id : files)
    {
      if (!isFirst)
        query << ", ";
      query << convertIdToVerbatim<model::File>(id);
      isFirst = false;
    }

    query << ");";
  }

  odb::result<model::BuildActionIdDetachedView> actionIds(
    _ctx.db->query<model::BuildActionIdDetachedView>(query.str()));
  std::for_each(actionIds.begin(), actionIds.end(), [&actions](auto& id)
  {
    actions.emplace(actions.end(), id.id);
  });
}

std::vector<std::pair<std::string, std::string>> SymbolClusterer::statistics() const
{
  odb::transaction t(_ctx.db->begin());

  // To build the statistics we will use the previously put together
  // mangled name equivalence set markers at some places.
  SymbolClusterer::marker_map markers = getMangledNameMarkers();

  // Build the queries that will give us the statistics.
  std::vector<std::pair<std::string, std::string>> metricToQuery;

  metricToQuery.emplace_back(std::make_pair(
    "Total # of files", "SELECT COUNT(*) FROM \"File\""));
  metricToQuery.emplace_back(std::make_pair(
    "Total # of build actions", "SELECT COUNT(*) FROM \"BuildAction\""));

  {
    // Total count of functions
    metricToQuery.emplace_back(std::make_pair(
      "Function nodes (total, incl. calls)",
      R";(SELECT COUNT(*) FROM "CppAstNode"
WHERE "symbolType" = $SYMTYPE_FUNC$);"));

    metricToQuery.emplace_back(std::make_pair(
      "Function definitions",
      R";(SELECT COUNT(*) FROM "CppAstNode"
WHERE "symbolType" = $SYMTYPE_FUNC$
  AND "astType" = $ASTTYPE_DEF$);"));

    metricToQuery.emplace_back(std::make_pair(
      "Function declarations",
      R";(SELECT COUNT(*) FROM "CppAstNode"
WHERE "symbolType" = $SYMTYPE_FUNC$
  AND "astType" = $ASTTYPE_DECL$);"));

    // Ambiguous function nodes and mangled name groups
    std::string ambiguous = markers.at("function")();
    std::ostringstream query;
    query << "WITH \"funcs\" AS ("
          << ambiguous << ")\n"
          << "SELECT COUNT(*) FROM \"funcs\"";
    metricToQuery.emplace_back(std::make_pair(
      "Ambiguous function names", query.str()));

    query.str("");
    query << "WITH \"funcs\" AS ("
          << ambiguous << ")\n"
          << "SELECT CAST(SUM(\"count_all\") AS BIGINT) FROM \"funcs\"";
    metricToQuery.emplace_back(std::make_pair(
      "Ambiguous function nodes", query.str()));
  }

  {
    // Total count of types
    metricToQuery.emplace_back(std::make_pair(
      "Type nodes (total, incl. usage)",
      R";(SELECT COUNT(*) FROM "CppAstNode"
WHERE "symbolType" = $SYMTYPE_TYPE$);"));

    metricToQuery.emplace_back(std::make_pair(
      "Type definitions",
      R";(SELECT COUNT(*) FROM "CppAstNode"
WHERE "symbolType" = $SYMTYPE_TYPE$
  AND "astType" = $ASTTYPE_DEF$);"));

    metricToQuery.emplace_back(std::make_pair(
      "Type declarations",
      R";(SELECT COUNT(*) FROM "CppAstNode"
WHERE "symbolType" = $SYMTYPE_TYPE$
  AND "astType" = $ASTTYPE_DECL$);"));

    // Ambiguous type nodes and mangled name groups
    std::ostringstream query;
    query << R"end(WITH "types" AS (
SELECT
  "ALL"."count" AS "ALL",
  "Defs"."count" AS "defs",
  COALESCE("Defs"."mangledName", "ALL"."mangledName") AS "mangledName"
FROM
    (
      SELECT COUNT(*), "mangledName"
      FROM "CppAstNode"
      WHERE "symbolType" = 3
      GROUP BY "mangledName"
      ORDER BY "count" DESC
    ) "ALL"
  FULL OUTER JOIN
    (
      SELECT COUNT(*), "mangledName"
      FROM "CppAstNode"
      WHERE "symbolType" = 3
      AND "astType" = 3
      GROUP BY "mangledName"
      ORDER BY "count" DESC
    ) "Defs" ON "ALL"."mangledName" = "Defs"."mangledName"
WHERE "Defs"."count" != 1
))end";

    metricToQuery.emplace_back(std::make_pair(
      "Ambiguous type names",
      query.str() + std::string("\nSELECT COUNT(*) FROM \"types\"")));

    metricToQuery.emplace_back(std::make_pair(
      "Ambiguous type nodes",
      query.str() + std::string("\nSELECT CAST(SUM(\"ALL\") AS BIGINT)"
                                "FROM \"types\"")));
  }

  // Calculate the staticstics.
  std::vector<std::pair<std::string, std::string>> stats;

  for (auto& it : metricToQuery)
  {
    using namespace boost::algorithm;
    replace_all(it.second, "$SYMTYPE_FUNC$", std::to_string(
      static_cast<size_t>(model::CppAstNode::SymbolType::Function)));
    replace_all(it.second, "$SYMTYPE_TYPE$", std::to_string(
      static_cast<size_t>(model::CppAstNode::SymbolType::Type)));
    replace_all(it.second, "$ASTTYPE_DECL$", std::to_string(
      static_cast<size_t>(model::CppAstNode::AstType::Declaration)));
    replace_all(it.second, "$ASTTYPE_DEF$", std::to_string(
      static_cast<size_t>(model::CppAstNode::AstType::Definition)));

    odb::result<model::SingleCountView> value =
      _ctx.db->query<model::SingleCountView>(it.second);

    if (value.empty())
      LOG(error) << "Couldn't query value for '" << it.first << "'";
    else
      stats.emplace_back(std::make_pair(it.first,
                                        std::to_string(value.begin()->value)));
  }

  return stats;
}

/** MangledNameMarker **/
void SymbolClusterer::MangledNameMarker::addReplaceRule(
  const std::string &search,
  const std::string &result)
{
  replaceRules[search] = result;
}

const std::string SymbolClusterer::MangledNameMarker::operator() () const
{
  using namespace boost::algorithm;
  std::string result = nonReplacedString;

  for (auto rule : replaceRules)
    replace_all(result, rule.first, rule.second);

  return result;
}

} // parser
} // cc


namespace
{

#if defined(DATABASE_PGSQL) || defined(DATABASE_SQLITE)
template <typename Object, typename ID>
std::string convertIdToVerbatim(const ID& id)
{
  using namespace odb;

  // ODB generates a custom type with the second template argument id_common,
  // which public-derives from the actual implementing class (id_pgsql or
  // id_sqlite as second argument). This class contains the type below,
  // which is the ACTUAL id type used by the database engine.
  decltype(access::object_traits_impl<Object, id_common>
           ::id_image_type::id_value) val = id;
  return std::to_string(val);
}
#else
#pragma GCC warning "\
Currently selected database engine CodeCompass is built with does not have an \
ID converter assigned. The fallback ID converter method might not work \
properly. This usually results in SymbolClusterer working with less accuracy!"

template <typename ID>
std::string convertIdToVerbatim(const ID& id)
{
  return std::to_string(id);
}
#endif

} // (anonymous)