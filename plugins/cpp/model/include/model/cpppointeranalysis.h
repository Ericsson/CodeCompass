#ifndef CC_MODEL_CPPPOINTERANALYSIS_H
#define CC_MODEL_CPPPOINTERANALYSIS_H

#include <memory>
#include <set>

#include <util/hash.h>

namespace cc
{
namespace model
{

#pragma db object
struct CppPointerAnalysis
{
  typedef std::uint32_t Options_t;

  enum Options : Options_t
  {
    HeapObj      = 1 << 0,  /*!< Object which is allocated on the heap: `new`,
                                 `make_shared`, `make_unique`, `malloc`,
                                 `realloc`, etc. */
    StackObj     = 1 << 1,  /*!< Object which is allocated on the stack:
                                 `T x;`. */
    GlobalObject = 1 << 2,  /*!< Object, which is allocated globally. */
    NullPtr      = 1 << 3,  /*!< Null pointer: `nullptr` or `NULL`. */
    Reference    = 1 << 4,  /*!< Alias to a variable or a function. For
                                 example: `T& x = y;`. */
    FunctionCall = 1 << 5,  /*!< Function call. */
    Return       = 1 << 6,  /*!< Return statement. */
    Param        = 1 << 7,  /*!< Function parameters. */
    Member       = 1 << 8,  /*!< Data member of a struct, class, union. */
    Literal      = 1 << 9,  /*!< String literal. */
    InitList     = 1 << 10, /*!< Initialization list of a class, struct,
                                 array. */
    Array        = 1 << 11, /*!< Array variables. */
    Undefined    = 1 << 12  /*!< Non initalized type, undefined memory
                                 space. */
  };

  #pragma db value
  struct StmtSide
  {
    StmtSide() = default;

    StmtSide(
      std::uint64_t mangledNameHash_,
      Options_t options_,
      const std::string& operators_)
        : mangledNameHash(mangledNameHash_),
          options(options_),
          operators(operators_)
    {
    }

    std::uint64_t mangledNameHash = 0;
    Options_t options = 0;
    std::string operators;

    bool operator==(const StmtSide& rhs_) const
    {
      return mangledNameHash == rhs_.mangledNameHash;
    }

    bool operator<(const StmtSide& rhs_) const
    {
      return mangledNameHash < rhs_.mangledNameHash;
    }
  };

  #pragma db id
  int id;

  StmtSide lhs;
  StmtSide rhs;

  std::string toString() const;

  bool operator==(const CppPointerAnalysis& other_) const
  {
    return id == other_.id;
  }

#pragma db index member(lhs)
#pragma db index member(rhs)
};

typedef std::shared_ptr<CppPointerAnalysis> CppPointerAnalysisPtr;

inline std::string CppPointerAnalysis::toString() const
{
  return std::string("CppPointerAnalysis")
    .append("\nid = ").append(std::to_string(id))
    .append("\nlhs = ").append(std::to_string(lhs.mangledNameHash))
    .append("\nrhs = ").append(std::to_string(rhs.mangledNameHash));
}

inline std::uint64_t createIdentifier(const CppPointerAnalysis& panal_)
{
  return util::fnvHash(
    std::to_string(panal_.lhs.mangledNameHash) +
    std::to_string(panal_.rhs.mangledNameHash));
}

#pragma db view object(CppPointerAnalysis)
struct CppPointerAnalysisCount
{
  #pragma db column("count(" + CppPointerAnalysis::id + ")")
  std::size_t count;
};

struct Hash
{
  std::size_t operator()(const CppPointerAnalysis::StmtSide& s_) const
  {
    return s_.mangledNameHash;
  }
};

} // model
} // cc

#endif // CC_MODEL_CPPPOINTERANALYSIS_H
