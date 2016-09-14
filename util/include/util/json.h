#ifndef UTIL_JSON_H
#define UTIL_JSON_H

// TODO: boost::property_tree should be used instead of this JsonNode class.
// How to store numbers as values in boost::property_tree?

#include <vector>
#include <map>

namespace cc
{
namespace util
{

class JsonNode
{
public:
  enum Type { NUMBER, BOOLEAN, STRING, LIST, OBJECT };
  
  explicit JsonNode(int i);
  explicit JsonNode(unsigned u);
  explicit JsonNode(double d);
  explicit JsonNode(bool b);
  explicit JsonNode(const char* s);
  explicit JsonNode(const std::string& s);
  explicit JsonNode(const std::vector<JsonNode>& v);
  explicit JsonNode(const std::map<std::string, JsonNode>& m);

  JsonNode(const JsonNode& other);
  JsonNode(JsonNode&& other);

  JsonNode& operator=(const JsonNode& other);
  JsonNode& operator=(JsonNode&&);

  ~JsonNode();

  Type getType() const;
  
  template <typename T = double>
  T                            getNumberValue()           const { return num; }
  bool                         getBooleanValue()          const { return b;   }
  const std::string&           getStringValue()           const { return str; }
  const std::vector<JsonNode>& getListValue()             const { return lst; }
  const std::map<std::string, JsonNode>& getObjectValue() const { return obj; }
  
  template <typename T>
  const T& getValue() const
  {
    switch (type)
    {
      case NUMBER:  return reinterpret_cast<const T&>(num);
      case BOOLEAN: return reinterpret_cast<const T&>(b);
      case STRING:  return reinterpret_cast<const T&>(str);
      case LIST:    return reinterpret_cast<const T&>(lst);
      case OBJECT:  return reinterpret_cast<const T&>(obj);
    }
  }
  
  template <typename T>
  T& getValue()
  {
    return const_cast<T&>(const_cast<const JsonNode*>(this)->getValue<T>());
  }
  
  std::string print() const;

private:
  void copy(const JsonNode& other);
  void move(JsonNode&& other);

  Type type;
  
  union
  {
    double num;
    bool b;
    std::string str;
    std::vector<JsonNode> lst;
    std::map<std::string, JsonNode> obj;
  };
};

}
}

#endif