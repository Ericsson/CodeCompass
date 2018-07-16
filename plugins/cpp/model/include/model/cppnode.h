#ifndef CC_MODEL_CPPNODE_H
#define CC_MODEL_CPPNODE_H

#include <string>
#include <memory>

#include <model/file.h>

#include <util/hash.h>

namespace cc
{
namespace model
{

typedef std::uint64_t CppNodeId;

#pragma db object
struct CppNode
{
  #pragma db id
  CppNodeId id;

  #pragma db not_null
  FileId file_id;

  std::string toString() const;
};

typedef std::shared_ptr<CppNode> CppNodePtr;

inline std::string CppNode::toString() const
{
  return std::string("CppNode")
    .append("\nid = ").append(std::to_string(id))
    .append("\ndomainId = ").append(std::to_string(file_id));
}

inline std::uint64_t createIdentifier(const CppNode& node_)
{
  return node_.file_id;
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
