#include "opnames.h"

namespace cc
{
namespace parser
{

std::string unop2str(clang::UnaryOperatorKind kind)
{
  switch(kind)
  {
    case clang::UO_PostInc: return "++ (int)";
    case clang::UO_PostDec: return "-- (int)";
    case clang::UO_PreInc: return "++";
    case clang::UO_PreDec: return "--";
    case clang::UO_AddrOf: return "&";
    case clang::UO_Deref: return "*";
    case clang::UO_Plus: return "+";
    case clang::UO_Minus: return "-";
    case clang::UO_Not: return "~";
    case clang::UO_LNot: return "!";
    case clang::UO_Real: return "__real";
    case clang::UO_Imag: return "__imag";
    case clang::UO_Extension: return "__extension__";
    default: return "";
  }
}

std::string binop2str(clang::BinaryOperatorKind kind)
{
  switch(kind)
  {
    case clang::BO_PtrMemD: return ".*";
    case clang::BO_PtrMemI: return ".->";
    case clang::BO_Mul: return "*";
    case clang::BO_Div: return "/";
    case clang::BO_Rem: return "%";
    case clang::BO_Add: return "+";
    case clang::BO_Sub: return  "-";
    case clang::BO_Shl: return "<<";
    case clang::BO_Shr: return ">>";
    case clang::BO_LT: return "<";
    case clang::BO_GT: return ">";
    case clang::BO_LE: return "<=";
    case clang::BO_GE: return ">=";
    case clang::BO_EQ: return "==";
    case clang::BO_NE: return "!=";
    case clang::BO_And: return "&";
    case clang::BO_Xor: return "^";
    case clang::BO_Or: return "|";
    case clang::BO_LAnd: return "&&";
    case clang::BO_LOr: return "||";
    case clang::BO_Assign: return "=";
    case clang::BO_MulAssign: return "*=";
    case clang::BO_DivAssign: return "/=";
    case clang::BO_RemAssign: return "%=";
    case clang::BO_AddAssign: return "+=";
    case clang::BO_SubAssign: return "-=";
    case clang::BO_ShlAssign: return "<<=";
    case clang::BO_ShrAssign: return ">>=";
    case clang::BO_AndAssign: return "&=";
    case clang::BO_XorAssign: return "^=";
    case clang::BO_OrAssign: return "|=";
    case clang::BO_Comma: return ",";
    default: return "";
  }
}

} // parser
} // cc
