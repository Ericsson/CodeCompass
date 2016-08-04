#ifndef CC_SERVICE_LANGUAGE_LANGSERVICE_H
#define CC_SERVICE_LANGUAGE_LANGSERVICE_H

#include <string>
#include <limits>

#include <odb/lazy-ptr.hxx>

#include <model/language_common.h>
#include <model/position.h>

#include <model/cppastnode.h>

#include <language_types.h>

namespace cc
{
namespace service
{
namespace language
{


bool operator<(const model::Position& lhs, const model::Position& rhs);

bool operator==(const model::Position& lhs, const model::Position& rhs);

bool operator!=(const model::Position& lhs, const model::Position& rhs);

bool operator<(const model::Range& lhs, const model::Range& rhs);

bool operator==(const model::Range& lhs, const model::Range& rhs);

std::string baseName(const std::string& path, char ch = '/');

AstNodeInfo createAstNodeInfo(const model::CppAstNode& astNode);

template <typename AstNode>
std::string getFileloc(const AstNode& astNode)
{
  if (!astNode.location.file)
    return "";

  auto fileloc = baseName(astNode.location.file.load()->path);
  fileloc += ":" + std::to_string(astNode.location.range.start.line);
  fileloc += ":" + std::to_string(astNode.location.range.start.column);

  return fileloc;
}

template<class AstNode>
bool isClickable(const AstNode& astNode)
{
  return !astNode.mangledName.empty(); // || astNode.nodeclass & model::CppAstNode::Declaration;
}

template <>
inline bool isClickable(const model::CppAstNode& astNode)
{
  return astNode.visibleInSourceCode;
}

std::string textRange(const std::string& content,
                      const model::Range& range,
                      std::size_t limit = std::numeric_limits<std::size_t>::max());

std::string textRange(const std::string & content,
  std::size_t start_line, std::size_t start_col,
  std::size_t end_line, std::size_t end_col,
  std::size_t limit = std::numeric_limits<std::size_t>::max());

std::string escapeDot(std::string dotText);
std::string escapeHtml(std::string text);
std::string removeHtmlTags(const std::string& htmlText);

} // language
} // service
} // cc


#endif // CC_SERVICE_LANGUAGE_LANGSERVICE_H

