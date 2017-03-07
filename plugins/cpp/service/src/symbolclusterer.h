#ifndef CC_SERVICE_LANGUAGE_CPPSERVICE_SYMBOLCLUSTERER_H
#define CC_SERVICE_LANGUAGE_CPPSERVICE_SYMBOLCLUSTERER_H

#include <memory>
#include <vector>

#include <model/cppastnode.h>
#include <model/cppastnode-odb.hxx>

#include <util/odbtransaction.h>

#include <service/cppservice.h>

namespace cc
{
namespace service
{
namespace language
{

class SymbolClusterer
{
public:
  /**
   * @brief Measure the strength of the relationship between a node and some
   * candidates based on the parse-time calculated and stored clustering
   * information.
   *
   * @param db_          The server's database connection pointer.
   * @param from_        An AST node from which the navigation begins. This
   * node's clusters are checked against the candidate ones to decide the
   * strength of the relationship.
   * @param candidates_  A list of AST nodes whom have already been selected as
   * possible navigation endpoints (e.g. their mangled name matches the from_'s)
   * but need to have the relationship strength measured.
   * @param tagMap_      For each candidate, the relationship strength will
   * be indicated as a tag, which is appended into this parameter map.
   */
  static void addClusterTags(std::shared_ptr<odb::database> db_,
                             const model::CppAstNode& from_,
                             std::vector<model::CppAstNode>& candidates_,
                             CppServiceHandler::TagMap& tagMap_);
};

} // language
} // service
} // cc

#endif // CC_SERVICE_LANGUAGE_CPPSERVICE_SYMBOLCLUSTERER_H
