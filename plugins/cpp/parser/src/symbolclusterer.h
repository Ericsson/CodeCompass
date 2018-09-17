#ifndef CC_PARSER_SYMBOLCLUSTERER_H
#define CC_PARSER_SYMBOLCLUSTERER_H

#include <functional>
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

#include <boost/lambda/lambda.hpp>

#include <parser/parsercontext.h>

#include <util/logutil.h>
#include <util/threadpool.h>
#include <util/util.h>

#include <model/buildaction.h>
#include <model/file.h>
#include <model/cppclusterinfo.h>

namespace cc
{
namespace parser
{

/**
 * @brief Enhances symbol resolution accuracy in the C++ parser.
 *
 * SymbolClusterer is used to calculate a relationship between a given
 * source file and the target binaries it was compiled into. This way, when the
 * user intends to execute a query over a set of symbols (e.g. "jump to
 * definition"), the casual mangled-name based resolution can be made more
 * accurate as CodeCompass will be able to show the symbols that "could've been"
 * -actually- used there, based on the BuildAction information.
 */
class SymbolClusterer
{
public:
  /**
   * Represents the collection of build actions. A cluster is unambiguously
   * identified by the build action that creates the binary which forms the
   * cluster.
   */
  typedef std::vector<decltype(model::BuildAction::id)> actions;

  /**
   * The relation which maps files (containing symbols) to cluters.
   */
  typedef std::map<model::FileId, actions> cluster_collection;

  /**
   * @brief Create the SymbolClusterer algorithm's handler for the context.
   *
   * @note SymbolClusterer should only be ran AFTER the project's parsing
   * has successfully concluded.
   *
   * @param ctx_         The current parse context in which the clusterer
   * is executed.
   */
  SymbolClusterer(ParserContext& ctx_);

  /**
   * @brief Calculate the initial set: the file-cluster relation on which query
   * accuracy is expected to be improved.
   *
   * SymbolClusterer uses a set of rules to decide whether a file contains a
   * symbol which is "problematic". If it does, the file is marked for
   * clusterisation and this method will calculate an "initial" cluster for it.
   *
   * @param threadCount     Number of threads to use for the calculation.
   */
  cluster_collection retrieveInitialClusters(size_t threadCount);

  /**
   * @brief Perform clusterisation on the given initial cluster map.
   *
   * Clusterisation involved calculating the build action leaves for the
   * parameter build action set. Instead of mapping a source file to some build
   * actions, we map the files to the last build action (in the project's build
   * chain) their symbols appeared in - this is almost always a final binary
   * build target.
   *
   * @param clusterMap  The File-to-BuildAction-collection map. The result is
   * calculated in this data structure.
   * @param threadCount Number of threads to use for the calculation.
   */
  void performClusterisation(cluster_collection& clusterMap,
                             size_t threadCount);

  /**
   * On multithreaded systems, performClusterisation() is executed in a
   * parallel fashion to increase efficiency. Calling waitForClusters() blocks
   * the caller thread until the entire clusterisation process has finished.
   */
  void waitForClusters();

  /**
   * Calculate and return some statictics about the project, such as the number
   * of AST nodes that were ambiguous, etc.
   *
   * Return values are organised in a way that the pair's key is the metric's
   * name and the value is the value transformed to a string.
   */
  std::vector<std::pair<std::string, std::string>> statistics() const;

private:
  /**
   * Represents a verbatim SQL query which, when executed, results in a list of
   * mangled names.
   */
  class MangledNameMarker
  {
  public:
    /**
     * @param nonReplacedString  The marker's encapsulated query can contain
     * string parts that are replaced when needed.
     */
    MangledNameMarker(const std::string& nonReplacedString)
      : nonReplacedString(nonReplacedString) {}

    /**
     * @brief Add a replace rule to the list of replaces.
     *
     * @param search The string to replace.
     * @param result The string to replace the found occurences to.
     */
    void addReplaceRule(const std::string& search, const std::string& result);

    /**
     * Execute the registered replacement and return the resulting query string.
     */
    const std::string operator() () const;

  private:
    const std::string nonReplacedString;
    std::unordered_map<std::string, std::string> replaceRules;
  };

  /**
   * The marker map maps marker objects to some unique textual key.
   */
  typedef std::map<std::string, SymbolClusterer::MangledNameMarker> marker_map;

  /**
   * Helper function which returns a set of MangledNameMarker objects which
   * contain queries for SymbolClusterer::retrieveInitialClusters.
   */
  static marker_map getMangledNameMarkers();

  /**
   * Helper function which creates a verbatim SQL query string to use in
   * SymbolClusterer::retrieveInitialClusters.
   *
   * @param markers  The ::marker_map to use in
   * SymbolClusterer::retrieveInitialClusters.
   * @return The query string.
   */
  static std::string buildViewQuery(const marker_map& markers);

  /**
   * @brief Calculate the eliminated transitive closure of the
   * "file-is-built-into" relation.
   *
   * This method uses SymbolClusterer::calculateReachablePropagation to walk
   * the tree spanned from the build information retrieved by CodeCompass to
   * calculate for a file which build actions (and thus, clusters) are
   * the ones the file actually belongs to.
   *
   * @see SymbolClusterer::calculateReachablePropagation for the properties of
   * the eliminated transitive closure.
   *
   * @param actions   The set of clusters the file belongs to. This variable
   * will be updated in place with the result.
   */
  void calculateBuildActionLeaves(actions& actions);

  /**
   * @brief Helper function for SymbolClusterer::retrieveInitialClusters to
   * calculate the initial cluster for a header.
   *
   * Header files are only stored in the database as includer->included pairs
   * (cc::model::CppHeaderInclusion). Due to this, we must manually calculate
   * the inclusion chain for a header to see which source files include the
   * given header in particular.
   *
   * After that, the initial cluster set for the header will be the
   * header-includer source files' initial set.
   *
   * @param file      The header file's ID record.
   * @param actions   The set of clusters the file belongs to. This variable
   * will be updated in place with the result.
   */
  void propagateHeaderIntoInitialCluster(const model::FileId& file,
                                         actions& actions);

  /**
   * @brief Calculate the eliminated transitive closure of a relation stored
   * in the database.
   *
   * This method walks the database and calculates the reachable endpoints (the
   * eliminated transitive closure) of a relation which is represented in the
   * database.
   *
   * Transitivity: if the results's output is further used as input in a
   * subsequent step, this subsequent step also belongs to the result.
   * Eliminated: Due to the way how symbols are propagated through subsequent
   * build outputs, the "intermediate" nodes are no longer relevant once an
   * expansion happens. Thus, they are removed from the result set.
   *
   * View must be an 'odb db view' struct which has two data members, a
   * 'key' of type GroupKey and a 'value' of type odb::nullable<GroupKey>.
   *
   * Result must be a Collection of GroupKey, satisfying that it has a
   * ForwardIterator and it supports the emplacement of any number of elements.
   *
   * QueryBuilder must be a BinaryFunction of Result::const_iterator returning
   * std::string.
   *
   * ExpandPredicate is a BinaryPredicate of two variables of GroupKey type.
   *
   * @note This method guarantess no exception safety. If the underyling
   * database engine or the container throws, the value of result is left
   * unspecified.
   *
   * @note This method must be called within an odb::transaction, but does NOT
   *       access the database for writing.
   *
   * @tparam GroupKey        The type of the key by which we can transitively
   * continue with calculating the relation.
   * @tparam View            The database view object which is used to store the
   * query results in.
   * @tparam Result          The result collection's type.
   * @tparam QueryBuilder    The functor for building the verbatim query used
   * in calculating the result.
   * @tparam ExpandPredicate The predicate for deciding whether or not an
   * expansion in the transitive relation should take place.
   * @param result        Initially result is a collection of keys from which
   * calculation starts. At return, the output of the algorithm is stored inű
   * this variable.
   * @param queryBuilder  A functor which builds the query executed by the
   * database engine to calculate subsequent transitive steps.
   * @param shouldExpand  Predicate function which decides whether or not
   * the found possible expansion found in the result set should be considered
   * a valid expansion path. This predicate can be used to prevent infinite
   * loops, as is the case in SymbolClusterer::propagateHeaderIntoInitialCluster.
   */
  template <typename GroupKey,
            typename View,
            typename Result,
            typename QueryBuilder,
            typename ExpandPredicate = std::function<bool (GroupKey, GroupKey)>>
  void calculateReachablePropagation(Result& result,
                                     QueryBuilder queryBuilder,
                                     ExpandPredicate shouldExpand =
                                       boost::lambda::constant(true)) const;

  /**
   * The context object in which the parser is running.
   */
  ParserContext& _ctx;

  /**
   * Reference wrapper over elements of SymbolClusterer::cluster_collection.
   */
  typedef std::reference_wrapper<cluster_collection::value_type> cluster_vref;

  /**
   * The type for the cluster calculating thread pool in
   * SymbolClusterer::performClusterisation.
   */
  typedef util::JobQueueThreadPool<cluster_vref> calculator_pool_type;

  /**
   * A thread pool for SymbolClusterer::performClusterisation.
   */
  std::unique_ptr<calculator_pool_type> _calculatorPool;
};

} // parser
} // cc

#endif // CC_PARSER_SYMBOLCLUSTERER_H
