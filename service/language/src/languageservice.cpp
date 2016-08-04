#include <regex>

#include <boost/log/trivial.hpp>

#include <model/file.h>
#include <model/file-odb.hxx>

#include <languageservice/languageservice.h>

namespace cc
{
namespace service
{
namespace language
{


bool operator<(const model::Position& lhs, const model::Position& rhs)
{
  return lhs.line < rhs.line ||
    (lhs.line == rhs.line && lhs.column < rhs.column);
}

bool operator==(const model::Position& lhs, const model::Position& rhs)
{
  return lhs.line == rhs.line && lhs.column == rhs.column;
}

bool operator!=(const model::Position& lhs, const model::Position& rhs)
{
  return !(lhs == rhs);
}

bool operator<(const model::Range& lhs, const model::Range& rhs)
{
  // Range lhs is less than range rhs, if rhs is comletely contains lhs.

  if(lhs.start == rhs.start)
    return lhs.end < rhs.end;

  if(lhs.end == rhs.end)
    return rhs.start < lhs.start;

  return rhs.start < lhs.start && lhs.end < rhs.end;
}

bool operator==(const model::Range& lhs, const model::Range& rhs)
{
  return rhs.start == lhs.start && lhs.end == rhs.end;
}

std::string baseName(const std::string& path, char ch)
{
  auto idx = path.find_last_of(ch);

  return path.substr(idx + 1);
}

std::string textRange(const std::string& content, const model::Range& range, std::size_t limit)
{
  return std::move(textRange(content, range.start.line, range.start.column,
    range.end.line, range.end.column, limit));
}

std::string textRange(const std::string & content,
  std::size_t start_line, std::size_t start_col,
  std::size_t end_line, std::size_t end_col, std::size_t limit)
{
  std::istringstream iss(content);
  std::string lineStr;

  std::string res;
  std::size_t i = 0;
  while(++i <= end_line && res.size() < limit)
  {
    std::getline(iss, lineStr);

    if(start_line < end_line)
    {
      if(start_line == i)
      {
        res += lineStr.substr(start_col - 1) + '\n';
      }
      else
      if(end_line == i)
      {
        res += lineStr.substr(0, end_col - 1) + '\n';
      }
      else
      if(start_line < i && i < end_line)
      {
        res += lineStr + '\n';
      }
    }
    else
    if(start_line == i)
    {
      res = lineStr.substr(start_col - 1, end_col - start_col);
    }
  }

  if(res.size() > limit)
  {
    res.resize(limit);
  }

  BOOST_LOG_TRIVIAL(info) << "res == " << res;
  return res;
}

AstNodeInfo createAstNodeInfo(
  const model::CppAstNode& astNode)
{
  BOOST_LOG_TRIVIAL(info) << "creating AstNodeInfo for" << astNode.astValue;

  AstNodeInfo ret;

  ret.astNodeId.astNodeId = std::to_string(astNode.id);
  ret.astNodeType = model::CppAstNode::symbolTypeToString(astNode.symbolType);

  if(astNode.location.file)
  {
    ret.range.file.fid = std::to_string(astNode.location.file.object_id());
    
    astNode.location.file.load()->content.load();
    const auto& content = astNode.location.file->content->content;

    model::Range range = astNode.location.range;
    range.start.column = 1;
    range.end.column = std::string::npos;

    const auto ROW_SIZE = 100;

    ret.astNodeSrcText = textRange(content, range, ROW_SIZE);
  }

  ret.astNodeValue = astNode.astValue;

  ret.range.range.startpos.line = astNode.location.range.start.line;
  ret.range.range.startpos.column = astNode.location.range.start.column;

  ret.range.range.endpos.line = astNode.location.range.end.line;
  ret.range.range.endpos.column = astNode.location.range.end.column;

  ret.documentation = "Documentation";

  BOOST_LOG_TRIVIAL(info) << "created AstNodeInfo for" << astNode.astValue;

  return ret;
}

std::string escapeDot(std::string dotText)
{
  for (std::size_t i = 0; i < dotText.length(); ++i)
  {
    switch (dotText[i])
    {
      case '<':
      case '>':
      {
        dotText.insert(i, "\\");
        ++i;
        break;
      }
      default:
        break;
    }
  }

  return dotText;
}

std::string escapeHtml(std::string text)
{
  for (std::size_t i = 0, n = text.length(); i < n ; ++i)
  {
    switch (text[i])
    {
    case '<':
      {
        text[i] = ';';
        text.insert(i, "t");
        text.insert(i, "l");
        text.insert(i, "&");
        ++i;
      }
      break;

    case '>':
      {
        text[i] = ';';
        text.insert(i, "t");
        text.insert(i, "g");
        text.insert(i, "&");
        ++i;
      }
      break;

    default:
      break;
    }
  }

  return text;
}

std::string removeHtmlTags(const std::string& htmlText)
{
  std::regex htmlTag("\\<[^\\>]*\\>|[\t]| {2,}");
  std::regex newLine("\n+");

  return std::regex_replace(std::regex_replace(htmlText, htmlTag, ""),
    newLine, " ");
}

} // language
} // service
} // cc
