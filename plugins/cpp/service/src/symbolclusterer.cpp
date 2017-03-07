#include <model/buildaction.h>
#include <model/cppclusterinfo.h>
#include <model/cppclusterinfo-odb.hxx>

#include "symbolclusterer.h"

namespace cc
{
namespace service
{
namespace language
{

void SymbolClusterer::addClusterTags(
  std::shared_ptr<odb::database> db_,
  const model::CppAstNode& from_,
  std::vector<model::CppAstNode>& candidates_,
  CppServiceHandler::TagMap& tagMap_)
{
  typedef decltype(model::BuildAction::id) BuildActionId;
  typedef odb::query<model::CppClusterInfo> ClusterQuery;

  auto getClustersForAstNode =
    [&db_](const model::CppAstNode& astNode) -> std::set<BuildActionId>
    {
      std::set<BuildActionId> set;
      auto actions = db_->query<model::CppClusterInfo>(
        ClusterQuery::file == astNode.location.file.object_id());

      std::for_each(actions.begin(), actions.end(), [&set]
        (const model::CppClusterInfo &info)
        {
          set.emplace(info.action.object_id());
        });

      return set;
    };

  std::set<BuildActionId> inputSet = getClustersForAstNode(from_);

  // Prevent marking certain inputs weak automatically when there
  // is no information available. Leave candidates_ and tagMap_ untouched.
  if (inputSet.empty())
    return;

  std::vector<model::CppAstNode> strong, normal, weak;
  strong.reserve(candidates_.size());
  normal.reserve(candidates_.size());
  weak.reserve(candidates_.size());

  auto it = candidates_.begin();
  auto end = candidates_.end();
  while (it != end)
  {
    std::set<BuildActionId> targetSet = getClustersForAstNode(*it);

    if (targetSet.empty())
    {
      // This is a rare case when the candidate's clusters were not calculated,
      // but it might happen. In this case, we don't append any special tag.
      normal.push_back(*it);

      ++it;
      continue;
    }

    // We don't use a full std::set_intersection here as finding the first
    // match is more than enough for us to unequivocally say that there is a
    // strong match.
    bool clusterReachable = false;
    {
      typename std::set<BuildActionId>::iterator input = inputSet.begin(),
        target = targetSet.begin();

      while (input != inputSet.end() && target != targetSet.end())
      {
        if (*input < *target)
          ++input;
        else
        {
          if (*target >= *input)
          {
            clusterReachable = true;
            break;
          }

          ++target;
        }
      }
    }

    if (clusterReachable)
    {
      /**
       * A strong match means that the clusterer at parse time, see
       * CppParser::SymbolClusterer, has resolved a static link between the
       * from_ symbol and the current candidate (*it).
       *
       * E.g. a function definition candidate was built into the same binary
       * with a function call. If this is the case, the call will call into that
       * particular definition, due to how the language rules are laid out
       * (see ODR).
       */
      tagMap_[it->id].emplace_back("match-strong");
      strong.push_back(*it);
    }
    else
    {
      /**
       * A weak match means that the clusterer calculated information at parse
       * time, but there isn't a common cluster between from_ and the candidate.
       *
       * E.g. the candidate is a function definition not build into the same
       * binary with a function call (even though the call APPEARS TO refer
       * to the definition, their mangled names match), it is highly unlikely
       * (at least in terms of static linkage) that this call ends up executing
       * code from that definition.
       */
      tagMap_[it->id].emplace_back("match-weak");
      weak.push_back(*it);
    }

    ++it;
  }

  // The candidates_ vector will be sorted so that strong matches are on the top.
  candidates_.clear();
  candidates_.insert(candidates_.end(), strong.begin(), strong.end());
  candidates_.insert(candidates_.end(), normal.begin(), normal.end());
  candidates_.insert(candidates_.end(), weak.begin(), weak.end());
}


} // language
} // service
} // cc