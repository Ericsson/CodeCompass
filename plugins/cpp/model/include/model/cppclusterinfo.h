#ifndef CC_MODEL_CPPCLUSTERINFO_H
#define CC_MODEL_CPPCLUSTERINFO_H

#include <vector>

#include <odb/lazy-ptr.hxx>
#include <odb/nullable.hxx>

#include <model/buildaction.h>
#include <model/file.h>

namespace cc
{
namespace model
{

/**
 * Represents a file's "built-into-cluster" relation. A file belongs to a given
 * cluster, if the file's contents has been built into the cluster. This build
 * can take arbitrary number of commands to conclude, forming a chain.
 */
#pragma db object no_id
struct CppClusterInfo
{
  #pragma db not_null
  odb::lazy_shared_ptr<File> file;

  // A cluster is uniquely identified by the build action which the resulting
  // binary forming the cluster is target of.
  #pragma db not_null
  odb::lazy_shared_ptr<BuildAction> action;

#ifndef NO_INDICES
  #pragma db index member(file)
#endif
};

#pragma db view
/**
 * Represents a view which maps a file (containing symbols) to an initial
 * cluster result from the database.
 */
struct FileToClusterView
{
  /**
   * The file which is mapped.
   */
  model::FileId file;

  /**
   * If file is a source file, it will be mapped to a BuildAction directly,
   * pointed by this pointer.
   */
  odb::nullable<decltype(BuildActionId::id)> build_action;

  /**
   * If file is a header file, it cannot directly be mapped to a BuildAction.
   * We need to indicate if the file is a header, so SymbolClusterer can map
   * the header the file that includes this header, which can yet again be a
   * header, or a source file -- from that we calculate the
   * cluster information to which the header should be mapped to.
   *
   * @see SymbolClusterer::retrieveInitialClusters for how this is handled.
   */
  odb::nullable<char> header_inclusion;
};

/**
 * Represents a database view containing a pair of build actions in the
 * expansion chain.
 * @see SymbolClusterer::calculateBuildActionLeaves
 */
#pragma db view
struct BuildActionExpandView
{
  decltype(BuildAction::id) key;

  #pragma db null
  odb::nullable<decltype(BuildAction::id)> value;
};

/**
 * Represents a database view containing a pair of files in the
 * preprocessor inclusion chain.
 * @see SymbolClusterer::propagateHeaderIntoInitialCluster
 */
#pragma db view
struct HeaderInclusionExpandView
{
  model::FileId key;

  #pragma db null
  odb::nullable<model::FileId> value;
};

/**
 * Represnets a database view containing BuildAction IDs, but unlike
 * model::BuildActionId, this one does not make ODB connect it to a BuildAction
 * instance, making it allow us to run native verbatim queries.
 * @see SymbolClusterer::propagateHeaderIntoInitialCluster
 */
#pragma db view
struct BuildActionIdDetachedView
{
    decltype(BuildAction::id) id;
};

}
}

#endif
