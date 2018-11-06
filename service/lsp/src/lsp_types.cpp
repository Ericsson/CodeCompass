#include <lspservice/lsp_types.h>

namespace cc
{
namespace service
{
namespace lsp
{

//--- ResponseError ---//

void ResponseError::writeNode(pt::ptree& node) const
{
  node.put("code", static_cast<int>(code));
  node.put("message", message);
}

//--- TextDocumentIdentifier ---//

void TextDocumentIdentifier::readNode(const pt::ptree& node)
{
  uri = node.get<DocumentUri>("uri");
}

void TextDocumentIdentifier::writeNode(pt::ptree& node) const
{
  node.put("uri", uri);
}

//--- Position ---//

void Position::readNode(const pt::ptree& node)
{
  line = node.get<int>("line");
  character = node.get<int>("character");
}

void Position::writeNode(pt::ptree& node) const
{
  node.put("line", line);
  node.put("character", character);
}

//--- Range ---//

void Range::readNode(const pt::ptree& node)
{
  start.readNode(node.get_child("start"));
  end.readNode(node.get_child("end"));
}

void Range::writeNode(pt::ptree& node) const
{
  node.put_child("start", start.createNode());
  node.put_child("end", end.createNode());
}

//--- Location ---//

void Location::readNode(const pt::ptree& node)
{
  uri = node.get<DocumentUri>("uri");
  range.readNode(node.get_child("range"));
}

void Location::writeNode(pt::ptree& node) const
{
  node.put("uri", uri);
  node.put_child("range", range.createNode());
}

//--- TextDocumentPositionParams ---//

void TextDocumentPositionParams::readNode(const pt::ptree& node)
{
  textDocument.readNode(node.get_child("textDocument"));
  position.readNode(node.get_child("position"));
}

void TextDocumentPositionParams::writeNode(pt::ptree& node) const
{
  node.put_child("textDocument", textDocument.createNode());
  node.put_child("position", position.createNode());
}

//--- ReferenceContext ---//

void ReferenceContext::readNode(const pt::ptree& node)
{
  includeDeclaration = node.get<bool>("includeDeclaration");
}

void ReferenceContext::writeNode(pt::ptree& node) const
{
  node.put("includeDeclaration", includeDeclaration);
}

//--- ReferenceParams ---//

void ReferenceParams::readNode(const pt::ptree& node)
{
  TextDocumentPositionParams::readNode(node);
  context.readNode(node.get_child("context"));
}

void ReferenceParams::writeNode(pt::ptree& node) const
{
  TextDocumentPositionParams::writeNode(node);
  node.put_child("context", context.createNode());
}

//--- DiagramTypeParams ---//

void DiagramTypeParams::readNode(const pt::ptree& node)
{
  textDocument.readNode(node.get_child("textDocument"));

  if (auto item = node.get_child_optional("position"))
  {
    position = Position();
    position->readNode(item.get());
  }
}

void DiagramTypeParams::writeNode(pt::ptree& node) const
{
  node.put_child("textDocument", textDocument.createNode());
  if (position)
    node.put_child("position", position->createNode());
}

//--- DiagramParams ---//

void DiagramParams::readNode(const pt::ptree& node)
{
  DiagramTypeParams::readNode(node);
  diagramType = node.get<std::string>("diagramType");
}

void DiagramParams::writeNode(pt::ptree& node) const
{
  DiagramTypeParams::writeNode(node);
  node.put("diagramType", diagramType);
}

//--- CompletionItem ---//

void CompletionItem::readNode(const pt::ptree& node)
{
  label = node.get<std::string>("label");
  if (auto item = node.get_optional<int>("kind"))
    kind = item;
  if (auto item = node.get_optional<std::string>("detail"))
    detail = item;
  if (auto item = node.get_optional<std::string>("documentation"))
    documentation = item;
  if (auto item = node.get_optional<std::string>("data"))
    data = item;
}

void CompletionItem::writeNode(pt::ptree& node) const
{
  node.put("label", label);
  if (kind)
    node.put("kind", kind);
  if (detail)
    node.put("detail", detail);
  if (documentation)
    node.put("documentation", documentation);
  if (data)
    node.put("data", data);
}

//--- CompletionList ---//

void CompletionList::writeNode(pt::ptree& node) const
{
  node.put("isIncomplete", isIncomplete);

  pt::ptree itemsNode;
  for (const CompletionItem& item : items)
  {
    itemsNode.push_back(std::make_pair("", item.createNode()));
  }
  node.put_child("items", itemsNode);
}

} // lsp
} // service
} // cc