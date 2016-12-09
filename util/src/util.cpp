#include <iostream>
#include <ctime>
#include <sstream>
#include <util/util.h>

namespace cc
{
namespace util
{

std::string getCurrentDate()
{
  std::time_t rawtime;
  struct tm* timeinfo;

  std::time(&rawtime);
  timeinfo = std::localtime(&rawtime);

  char output[42];
  std::strftime(output, 42, "%c", timeinfo);

  return {output};
}

std::string textRange(
  const std::string& text_,
  std::size_t startLine_, std::size_t startCol_,
  std::size_t endLine_, std::size_t endCol_)
{
  std::string res;

  if (startLine_ == static_cast<std::size_t>(-1) ||
      startCol_  == static_cast<std::size_t>(-1) ||
      endLine_   == static_cast<std::size_t>(-1) ||
      endCol_    == static_cast<std::size_t>(-1))
    return res;

  std::istringstream iss(text_);
  std::string lineStr;

  for (std::size_t i = 1; i <= endLine_; ++i)
  {
    std::getline(iss, lineStr);

    if (startLine_ < endLine_)
    {
      if (startLine_ == i)
        res += lineStr.substr(startCol_ - 1) + '\n';
      else if (endLine_ == i)
        res += lineStr.substr(0, endCol_ - 1);
      else if (startLine_ < i && i < endLine_)
        res += lineStr + '\n';
    }
    else if (startLine_ == i)
      res = lineStr.substr(startCol_ - 1, endCol_ - startCol_);
  }

  return res;
}

std::string escapeHtml(const std::string& str_)
{
  std::string ret;

  for (char c : str_)
    switch (c)
    {
      case '<': ret += "&lt;";  break;
      case '>': ret += "&gt;";  break;
      case '&': ret += "&amp;"; break;
      default : ret += c;       break;
    }

  return ret;
}

}
}
