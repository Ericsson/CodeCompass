#include <sstream>
#include <vector>

#include <clang/AST/Comment.h>
#include <clang/AST/CommentVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

#include <util/logutil.h>

#include "doccommentformatter.h"

using namespace clang;
using namespace clang::comments;

namespace
{

/**
 * Separate parts of a FullComment.
 */
struct FullCommentParts {
  /**
   * Take a full comment apart and initialize members accordingly.
   */
  FullCommentParts(const FullComment* c_,
                   const CommandTraits& traits_);

  const BlockContentComment *brief; /*!< A paragraph that serves as a brief
    description. For more information see:
    http://www.stack.nl/~dimitri/doxygen/manual/commands.html#cmdbrief*/

  const BlockContentComment *headerfile;
  const ParagraphComment *firstParagraph;
  std::vector<const BlockCommandComment *> returns;
  std::vector<const ParamCommandComment *> params;
  std::vector<const TParamCommandComment *> tParams;
  llvm::TinyPtrVector<const BlockCommandComment *> exceptions;
  std::vector<const BlockContentComment *> miscBlocks;
};

FullCommentParts::FullCommentParts(
  const FullComment *c_,
  const CommandTraits &traits_)
  : brief(nullptr),
    headerfile(nullptr),
    firstParagraph(nullptr)
{
  for (Comment::child_iterator it = c_->child_begin(), end = c_->child_end();
       it != end; ++it)
  {
    const Comment *child = *it;

    if (!child)
      continue;

    switch (child->getCommentKind())
    {
      case Comment::NoCommentKind:
        continue;

      case Comment::ParagraphCommentKind:
      {
        const ParagraphComment *pc = llvm::cast<ParagraphComment>(child);

        if (pc->isWhitespace())
          break;

        if (!firstParagraph)
          firstParagraph = pc;

        miscBlocks.push_back(pc);
        break;
      }

      case Comment::BlockCommandCommentKind:
      {
        const BlockCommandComment *bcc = llvm::cast<BlockCommandComment>(child);
        const CommandInfo *Info = traits_.getCommandInfo(bcc->getCommandID());
        if (!brief && Info->IsBriefCommand) {
          brief = bcc;
          break;
        }
        if (!headerfile && Info->IsHeaderfileCommand)
        {
          headerfile = bcc;
          break;
        }
        if (Info->IsReturnsCommand)
        {
          returns.push_back(bcc);
          break;
        }
        if (Info->IsThrowsCommand)
        {
          exceptions.push_back(bcc);
          break;
        }
        miscBlocks.push_back(bcc);
        break;
      }

      case Comment::ParamCommandCommentKind:
      {
        const ParamCommandComment *pcc = llvm::cast<ParamCommandComment>(child);
        if (!pcc->hasParamName())
          break;

        if (!pcc->isDirectionExplicit() && !pcc->hasNonWhitespaceParagraph())
          break;

        params.push_back(pcc);
        break;
      }

      case Comment::TParamCommandCommentKind:
      {
        const TParamCommandComment *tpcc =
          llvm::cast<TParamCommandComment>(child);

        if (!tpcc->hasParamName())
          break;

        if (!tpcc->hasNonWhitespaceParagraph())
          break;

        tParams.push_back(tpcc);
        break;
      }

      case Comment::VerbatimBlockCommentKind:
        miscBlocks.push_back(cast<BlockCommandComment>(child));
        break;

      case Comment::VerbatimLineCommentKind:
      {
        const VerbatimLineComment *vlc = llvm::cast<VerbatimLineComment>(child);
        const CommandInfo *Info = traits_.getCommandInfo(vlc->getCommandID());

        if (!Info->IsDeclarationCommand)
          miscBlocks.push_back(vlc);

        break;
      }

      default:
        LOG(warning) << "AST node of this kind can't be a child of a "
                        "FullComment!";
    }
  }
}

// https://llvm.org/svn/llvm-project/cfe/trunk/lib/Index/CommentToXML.cpp
class CommentToHTMLConverter :
    public ConstCommentVisitor<CommentToHTMLConverter> {
public:
  CommentToHTMLConverter(
    const FullComment* fullComment_,
    const CommandTraits& traits_,
    std::stringstream& res_)
    : _fullComment(fullComment_),
      _traits(traits_),
      _res(res_)
  { }

  /*
   * This function visits only the documentation part of the comment.
   */
  void visitTextComment(const TextComment* c_);

  /**
   * This function visits visualisation blocks such as '\a', '\b', '\c' etc.
   */
  void visitInlineCommandComment(const InlineCommandComment* c_);

  /**
   * This function visits html commands' start tags (e.g.: <b>, <code>, <div>
   * etc.) in the documentation.
   * For more information see:
   * http://www.stack.nl/~dimitri/doxygen/manual/htmlcmds.html
   */
  void visitHTMLStartTagComment(const HTMLStartTagComment* c_);

  /**
   * This function visits html commands' end tags (e.g.: </b>, </code>, </div>
   * etc.) in the documentation.
   * For more information see:
   * http://www.stack.nl/~dimitri/doxygen/manual/htmlcmds.html
   */
  void visitHTMLEndTagComment(const HTMLEndTagComment* c_);

  /**
   * This functions visit the text after an attribute like '@param'.
   */
  void visitParagraphComment(const ParagraphComment* c_);

  /**
   * This function visits the block commands such as \brief, \date, \author etc.
   */
  void visitBlockCommandComment(const BlockCommandComment* c_);

  /**
   * This function visits the `@param` attributes of the comment.
   */
  void visitParamCommandComment(const ParamCommandComment* c_);

  /**
   * This function visits '\tparam' section.
   */
  void visitTParamCommandComment(const TParamCommandComment* c_);

  /**
   * This function visits the '@verbatim' block comments.
   */
  void visitVerbatimBlockComment(const VerbatimBlockComment* c_);

  /**
   * This function visits the '@verbatim' line comments.
   */
  void visitVerbatimLineComment(const VerbatimLineComment* c_);

  /**
   * This function the main visitor. It will call the other visitor functions.
   */
  void visitFullComment(const FullComment* c_);

  /**
   * This is a helper function which visits non standalone paragraph comments.
   */
  void visitNonStandaloneParagraphComment(const ParagraphComment* c_);

  /**
   * This function escapes html elements from parameter and appends the escaped
   * string to the result.
   */
  void appendToResultWithHTMLEscaping(const clang::StringRef& str_);

private:
  const FullComment* _fullComment;
  const CommandTraits& _traits;
  std::stringstream& _res;
};

}

void CommentToHTMLConverter::appendToResultWithHTMLEscaping(
  const clang::StringRef& str_)
{
  for (clang::StringRef::iterator it = str_.begin(), end = str_.end();
    it != end; ++it)
  {
    const char C = *it;
    switch (C)
    {
      case '&':
        _res << "&amp;";
        break;

      case '<':
        _res << "&lt;";
        break;

      case '>':
        _res << "&gt;";
        break;

      case '"':
        _res << "&quot;";
        break;

      case '\'':
        _res << "&#39;";
        break;

      case '/':
        _res << "&#47;";
        break;

      default:
        _res << C;
        break;
    }
  }
}

void CommentToHTMLConverter::visitTextComment(const TextComment* c_)
{
  _res << "<div class=\"comment\">";
  appendToResultWithHTMLEscaping(c_->getText());
  _res << "</div>";
}

void CommentToHTMLConverter::visitInlineCommandComment(
  const InlineCommandComment* c_)
{
  //--- Nothing to render if argument is empty. ---//

  if (!c_->getNumArgs())
    return;

  StringRef arg0 = c_->getArgText(0);
  if (arg0.empty())
    return;

  switch (c_->getRenderKind())
  {
    case InlineCommandComment::RenderNormal:
      for (unsigned i = 0, e = c_->getNumArgs(); i != e; ++i)
      {
        appendToResultWithHTMLEscaping(c_->getArgText(i));
        _res << " ";
      }
      return;

    case InlineCommandComment::RenderBold:
      assert(c_->getNumArgs() == 1);
      _res << "<b>";
      appendToResultWithHTMLEscaping(arg0);
      _res << "</b>";
      return;

    case InlineCommandComment::RenderMonospaced:
      assert(c_->getNumArgs() == 1);
      _res << "<tt>";
      appendToResultWithHTMLEscaping(arg0);
      _res<< "</tt>";
      return;

    case InlineCommandComment::RenderEmphasized:
      assert(c_->getNumArgs() == 1);
      _res << "<em>";
      appendToResultWithHTMLEscaping(arg0);
      _res << "</em>";
      return;
  }
}

void CommentToHTMLConverter::visitHTMLStartTagComment(
  const HTMLStartTagComment* c_)
{
  _res << "<" << c_->getTagName().str();

  if (c_->getNumAttrs())
    for (unsigned i = 0, e = c_->getNumAttrs(); i != e; i++)
    {
      _res << " ";
      const HTMLStartTagComment::Attribute &Attr = c_->getAttr(i);
      _res << Attr.Name.str();
      if (!Attr.Value.empty())
        _res << "=\"" << Attr.Value.str() << "\"";
    }

  if (!c_->isSelfClosing())
    _res << ">";
  else
    _res << "/>";
}

void CommentToHTMLConverter::visitHTMLEndTagComment(const HTMLEndTagComment* c_)
{
  _res << "</" << c_->getTagName().str() << ">";
}

void CommentToHTMLConverter::visitParagraphComment(const ParagraphComment* c_)
{
  if (c_->isWhitespace())
    return;

  _res << "<div>";
  for (Comment::child_iterator it = c_->child_begin(), end = c_->child_end();
       it != end; ++it)
    visit(*it);

  _res << "</div>";
}

void CommentToHTMLConverter::visitBlockCommandComment(
  const BlockCommandComment* c_)
{
  const CommandInfo *info = _traits.getCommandInfo(c_->getCommandID());
  if (info->IsBriefCommand)
  {
    _res << "<div class=\"para-brief\">";
    visitNonStandaloneParagraphComment(c_->getParagraph());
    _res << "</div>";

    return;
  }

  if (info->IsReturnsCommand)
  {
    _res << "<div class=\"para-returns\">"
            "<span class=\"word-returns\">Returns</span> ";
    visitNonStandaloneParagraphComment(c_->getParagraph());
    _res << "</div>";

    return;
  }

  // We don't know anything about this command.  Just render the paragraph.
  visit(c_->getParagraph());
}

void CommentToHTMLConverter::visitParamCommandComment(
  const ParamCommandComment* c_)
{
  _res << "<dl class=\"param\"><dt>Parameter</dt><dd>";
  _res << "<div class=\"dir\">";
  switch (c_->getDirection()) {
    case ParamCommandComment::In:
      _res << "in";
      break;

    case ParamCommandComment::Out:
      _res << "out";
      break;

    case ParamCommandComment::InOut:
      _res << "in,out";
      break;
  }
  _res << "</div><div class=\"name\">";

  appendToResultWithHTMLEscaping(c_->isParamIndexValid()
    ? c_->getParamName(_fullComment)
    : c_->getParamNameAsWritten());

  _res << "</div>";

  visit(c_->getParagraph());

  _res << "</dd></dl>";
}

void CommentToHTMLConverter::visitTParamCommandComment(
  const TParamCommandComment* c_)
{
  _res << "<dl class=\"tparam\"><dt>Template parameter</dt><dd>";
  _res << "<div class=\"name\">";
  appendToResultWithHTMLEscaping(c_->getParamNameAsWritten());
  _res << "</div>";

  visitNonStandaloneParagraphComment(c_->getParagraph());
  _res << "</dd></dl>";
}

void CommentToHTMLConverter::visitVerbatimBlockComment(
  const VerbatimBlockComment* c_)
{
  unsigned numLines = c_->getNumLines();

  if (!numLines)
    return;

  _res << "<divre>";
  for (unsigned i = 0; i != numLines; ++i) {
    appendToResultWithHTMLEscaping(c_->getText(i));
    if (i + 1 != numLines)
      _res << '\n';
  }
  _res << "</pre>";
}

void CommentToHTMLConverter::visitVerbatimLineComment(
  const VerbatimLineComment* c_)
{
  _res << "<divre>";
  appendToResultWithHTMLEscaping(c_->getText());
  _res << "</pre>";
}

void CommentToHTMLConverter::visitFullComment(const FullComment* c_)
{
  FullCommentParts parts(c_, _traits);

  bool firstParagraphIsBrief = false;

  if (parts.headerfile)
    visit(parts.headerfile);

  if (parts.brief)
    visit(parts.brief);

  else if (parts.firstParagraph)
  {
    _res << "<div class=\"para-brief\">";
    visitNonStandaloneParagraphComment(parts.firstParagraph);
    _res << "</div>";
    firstParagraphIsBrief = true;
  }

  for (unsigned i = 0, e = parts.miscBlocks.size(); i != e; ++i)
  {
    const Comment *C = parts.miscBlocks[i];

    if (firstParagraphIsBrief && C == parts.firstParagraph)
      continue;

    visit(C);
  }

  if (parts.tParams.size() != 0)
  {
    _res << "<dl>";

    for (unsigned i = 0, e = parts.tParams.size(); i != e; ++i)
      visit(parts.tParams[i]);

    _res << "</dl>";
  }

  if (parts.params.size() != 0)
  {
    _res << "<dl>";

    for (unsigned i = 0, e = parts.params.size(); i != e; ++i)
      visit(parts.params[i]);

    _res << "</dl>";
  }

  if (parts.returns.size() != 0)
  {
    _res << "<div class=\"result-discussion\">";

    for (unsigned i = 0, e = parts.returns.size(); i != e; ++i)
      visit(parts.returns[i]);

    _res << "</div>";
  }
}

void CommentToHTMLConverter::visitNonStandaloneParagraphComment(
  const ParagraphComment* c_)
{
  if (!c_)
    return;

  for (Comment::child_iterator it = c_->child_begin(), end = c_->child_end();
       it != end; ++it)

  visit(*it);
}


namespace cc {
namespace parser {

using namespace clang::comments;

std::string DocCommentFormatter::format(
  clang::comments::FullComment *fc_,
  const clang::ASTContext& ctx_)
{
  std::stringstream res;
  CommentToHTMLConverter htmlConverter(fc_, ctx_.getCommentCommandTraits(), res);
  htmlConverter.visit(fc_);

  return res.str();
}

} // parser
} // cc
