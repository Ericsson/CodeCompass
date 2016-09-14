#include <algorithm>
#include <boost/algorithm/string/replace.hpp>
#include <util/json.h>

namespace cc
{
namespace util
{

JsonNode::JsonNode(int i)                                    : type(NUMBER),  num(i) {}
JsonNode::JsonNode(unsigned u)                               : type(NUMBER),  num(u) {}
JsonNode::JsonNode(double d)                                 : type(NUMBER),  num(d) {}
JsonNode::JsonNode(bool b)                                   : type(BOOLEAN), b(b)   {}
JsonNode::JsonNode(const char* s)                            : type(STRING),  str(s) {}
JsonNode::JsonNode(const std::string& s)                     : type(STRING),  str(s) {}
JsonNode::JsonNode(const std::vector<JsonNode>& v)           : type(LIST),    lst(v) {}
JsonNode::JsonNode(const std::map<std::string, JsonNode>& m) : type(OBJECT),  obj(m) {}

JsonNode::JsonNode(const JsonNode& other) { copy(other); }
JsonNode::JsonNode(JsonNode&& other) { move(std::move(other)); }

JsonNode& JsonNode::operator=(const JsonNode& other) {
  if (this == &other)
    return *this;

  this->~JsonNode();
  copy(other);
  return *this;
}

JsonNode& JsonNode::operator=(JsonNode&& other)
{
  if (this == &other)
    return *this;

  this->~JsonNode();
  move(std::move(other));
  return *this;
}

JsonNode::~JsonNode()
{
  switch (type)
  {
    case NUMBER:                                              break;
    case BOOLEAN:                                             break;
    case STRING: str.std::string::~basic_string();            break;
    case LIST:   lst.std::vector<JsonNode>::~vector();        break;
    case OBJECT: obj.std::map<std::string, JsonNode>::~map(); break;
  }
}

void JsonNode::copy(const JsonNode& other)
{
  type = other.type;

  switch (type)
  {
    case NUMBER:  num = other.num;                                       break;
    case BOOLEAN: b   = other.b;                                         break;
    case STRING:  new (&str) std::string(other.str);                     break;
    case LIST:    new (&lst) std::vector<JsonNode>(other.lst);           break;
    case OBJECT:  new (&obj) std::map<std::string, JsonNode>(other.obj); break;
  }
}

void JsonNode::move(JsonNode&& other)
{
  type = other.type;

  switch (type)
  {
    case NUMBER:  num = other.num;                                                  break;
    case BOOLEAN: b   = other.b;                                                    break;
    case STRING:  new (&str) std::string(std::move(other.str));                     break;
    case LIST:    new (&lst) std::vector<JsonNode>(std::move(other.lst));           break;
    case OBJECT:  new (&obj) std::map<std::string, JsonNode>(std::move(other.obj)); break;
  }
}

JsonNode::Type JsonNode::getType() const
{
  return type;
}

std::string JsonNode::print() const
{
  std::string result;
  std::string temp;

  switch (type)
  {
    case NUMBER:
      result = std::to_string(num);
      break;

    case BOOLEAN:
      result = b ? "true" : "false";
      break;

    case STRING:
      result = '"' + boost::replace_all_copy(str, "\"", "\\\"") + '"';
      break;

    case LIST:
      for (const JsonNode& child : lst)
        temp += child.print() + ',';
      temp.pop_back();
      result = '[' + temp + ']';
      break;

    case OBJECT:
      for (const auto& p : obj)
        temp
          += '"' + boost::replace_all_copy(p.first, "\"", "\\\"") + '"'
          +  ':'
          +  p.second.print()
          +  ',';
      temp.pop_back();
      result = '{' + temp + '}';
      break;
  }

  return result;
}

}
}