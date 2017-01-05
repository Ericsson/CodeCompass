#include <parser/traversal.h>

namespace cc
{
namespace parser
{

Traversal::~Traversal()
{
}

void Traversal::beforeTraverse(const OptionMap&, SourceManager&)
{
}

void Traversal::afterTraverse(SourceManager&)
{
}

Traversal::DirIterCallback Traversal::traverse(
  const std::string&,
  SourceManager&)
{
  return DirIterCallback();
}

void Traversal::endTraverse(
  const std::string&,
  SourceManager&)
{
}

} //parser
} //cc
