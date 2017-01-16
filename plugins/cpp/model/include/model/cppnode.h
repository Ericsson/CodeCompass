#ifndef CC_MODEL_CPPNODE_H
#define CC_MODEL_CPPNODE_H

#include <string>
#include <memory>

namespace cc
{
namespace model
{

typedef std::uint64_t CppNodeId;

#pragma db object
struct CppNode
{
  enum Domain
  {
    FILE,
    CPPASTNODE
  };

  #pragma db id
  CppNodeId id;

  #pragma db not_null
  Domain domain;

  #pragma db not_null
  std::string domainId;

  std::string toString() const;
};

typedef std::shared_ptr<CppNode> CppNodePtr;

inline std::string domainTypeToString(CppNode::Domain type_)
{
  switch (type_)
  {
    case CppNode::Domain::FILE: return "File";
    case CppNode::Domain::CPPASTNODE: return "CppAstNode";
  }

  return std::string();
}

inline std::string CppNode::toString() const
{
  return std::string("CppNode")
    .append("\nid = ").append(std::to_string(id))
    .append("\ndomain = ").append(domainTypeToString(domain))
    .append("\ndomainId = ").append(domainId);
}

typedef std::uint64_t CppNodeAttributeId;

#pragma db object
class CppNodeAttribute
{
public:
  #pragma db id
  CppNodeAttributeId id;

  #pragma db not_null
  std::shared_ptr<CppNode> node;

  #pragma db not_null
  std::string key;

  #pragma db null
  std::string value;
};

} // model
} // cc

#endif // CC_MODEL_CPPNODE_H
