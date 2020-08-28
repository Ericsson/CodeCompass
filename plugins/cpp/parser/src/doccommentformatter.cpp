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
struct FullCommentParts
{
  /**
   * Take a full comment apart and initialize members accordingly.
   */
  FullCommentParts(
    const FullComment* c_,
    const CommandTraits& traits_);

  const BlockContentComment* _brief; /*!< A paragraph that serves as a _brief
    description. For more information see:
    http://www.stack.nl/~dimitri/doxygen/manual/commands.html#cmd_brief*/

  const BlockContentComment* _headerfile;
  const ParagraphComment* _firstParagraph;
  std::vector<const BlockCommandComment*> _returns;
  std::vector<const ParamCommandComment*> _params;
  std::vector<const TParamCommandComment*> _tParams;
  llvm::TinyPtrVector<const BlockCommandComment*> _exceptions;
  std::vector<const BlockContentComment*> _miscBlocks;
};

FullCommentParts::FullCommentParts(
  const FullComment* c_,
  const CommandTraits& traits_)
    : _brief(nullptr),
      _headerfile(nullptr),
      _firstParagraph(nullptr)
{
  for (Comment::child_iterator it = c_->child_begin(), end = c_->child_end();
       it != end; ++it)
  {
    const Comment* child = *it;

    if (!child)
      continue;

    switch (child->getCommentKind())
    {
      case Comment::NoCommentKind:
        continue;

      case Comment::ParagraphCommentKind:
      {
        const ParagraphComment* pc = llvm::cast<ParagraphComment>(child);

        if (pc->isWhitespace())
          break;

        if (!_firstParagraph)
          _firstParagraph = pc;

        _miscBlocks.push_back(pc);
        break;
      }

      case Comment::BlockCommandCommentKind:
      {
        const BlockCommandComment* bcc = llvm::cast<BlockCommandComment>(child);
        const CommandInfo* Info = traits_.getCommandInfo(bcc->getCommandID());

        if (!_brief && Info->IsBriefCommand)
          _brief = bcc;
        else if (!_headerfile && Info->IsHeaderfileCommand)
          _headerfile = bcc;
        else if (Info->IsReturnsCommand)
          _returns.push_back(bcc);
        else if (Info->IsThrowsCommand)
          _exceptions.push_back(bcc);
        else
          _miscBlocks.push_back(bcc);

        break;
      }

      case Comment::ParamCommandCommentKind:
      {
        const ParamCommandComment* pcc = llvm::cast<ParamCommandComment>(child);

        if (!pcc->hasParamName())
          break;

        if (!pcc->isDirectionExplicit() && !pcc->hasNonWhitespaceParagraph())
          break;

        _params.push_back(pcc);
        break;
      }

      case Comment::TParamCommandCommentKind:
      {
        const TParamCommandComment* tpcc
          = llvm::cast<TParamCommandComment>(child);

        if (!tpcc->hasParamName())
          break;

        if (!tpcc->hasNonWhitespaceParagraph())
          break;

        _tParams.push_back(tpcc);
        break;
      }

      case Comment::VerbatimBlockCommentKind:
        _miscBlocks.push_back(cast<BlockCommandComment>(child));
        break;

      case Comment::VerbatimLineCommentKind:
      {
        const VerbatimLineComment* vlc = llvm::cast<VerbatimLineComment>(child);
        const CommandInfo* Info = traits_.getCommandInfo(vlc->getCommandID());

        if (!Info->IsDeclarationCommand)
          _miscBlocks.push_back(vlc);

        break;
      }

      default:
        LOG(debug)
          << "AST node of this kind can't be a child of a FullComment!";
    }
  }
}

// https://llvm.org/svn/llvm-project/cfe/trunk/lib/Index/CommentToXML.cpp
class CommentToMarkdownConverter
  : public ConstCommentVisitor<CommentToMarkdownConverter>
{
public:
  CommentToMarkdownConverter(
    const FullComment* fullComment_,
    const CommandTraits& traits_,
    std::stringstream& res_)
      : _fullComment(fullComment_),
        _traits(traits_),
        _res(res_)
  { }

  /**
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
   * This function visits the block commands such as \_brief, \date, \author etc.
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

private:
  const FullComment* _fullComment;
  const CommandTraits& _traits;
  std::stringstream& _res;
};

}

void CommentToMarkdownConverter::visitTextComment(const TextComment* c_)
{
  _res << c_->getText().ltrim().str() << ' ';
}

void CommentToMarkdownConverter::visitInlineCommandComment(
  const InlineCommandComment* c_)
{
  // Nothing to render if argument is empty.
  if (!c_->getNumArgs())
    return;

  StringRef arg0 = c_->getArgText(0);
  if (arg0.empty())
    return;

  switch (c_->getRenderKind())
  {
    case InlineCommandComment::RenderNormal:
      for (unsigned i = 0, e = c_->getNumArgs(); i != e; ++i)
        _res << c_->getArgText(i).str() << ' ';
      return;

    case InlineCommandComment::RenderBold:
      assert(c_->getNumArgs() == 1);
      _res << "**" << arg0.str() << "**";
      return;

    case InlineCommandComment::RenderMonospaced:
      assert(c_->getNumArgs() == 1);
      _res << '`' << arg0.str() << '`';
      return;

    case InlineCommandComment::RenderEmphasized:
      assert(c_->getNumArgs() == 1);
      _res << '*' << arg0.str() << '*';
      return;
  }
}

void CommentToMarkdownConverter::visitHTMLStartTagComment(
  const HTMLStartTagComment* c_)
{
  _res << '<' << c_->getTagName().str();

  if (c_->getNumAttrs())
    for (unsigned i = 0, e = c_->getNumAttrs(); i != e; i++)
    {
      _res << ' ';
      const HTMLStartTagComment::Attribute &Attr = c_->getAttr(i);
      _res << Attr.Name.str();
      if (!Attr.Value.empty())
        _res << "=\"" << Attr.Value.str() << '"';
    }

  if (!c_->isSelfClosing())
    _res << '>';
  else
    _res << "/>";
}

void CommentToMarkdownConverter::visitHTMLEndTagComment(
  const HTMLEndTagComment* c_)
{
  _res << "</" << c_->getTagName().str() << '>';
}

void CommentToMarkdownConverter::visitParagraphComment(
  const ParagraphComment* c_)
{
  if (c_->isWhitespace())
    return;

  for (Comment::child_iterator it = c_->child_begin(), end = c_->child_end();
       it != end; ++it)
    visit(*it);
}

void CommentToMarkdownConverter::visitBlockCommandComment(
  const BlockCommandComment* c_)
{
  const CommandInfo *info = _traits.getCommandInfo(c_->getCommandID());
  if (info->IsBriefCommand)
  {
    visitNonStandaloneParagraphComment(c_->getParagraph());
    return;
  }

  if (info->IsReturnsCommand)
  {
    visitNonStandaloneParagraphComment(c_->getParagraph());
    return;
  }

  // We don't know anything about this command. Just render the paragraph.
  visit(c_->getParagraph());
}

void CommentToMarkdownConverter::visitParamCommandComment(
  const ParamCommandComment* c_)
{
  switch (c_->getDirection())
  {
    case ParamCommandComment::In:    _res << "- *in*: ";     break;
    case ParamCommandComment::Out:   _res << "- *out*: ";    break;
    case ParamCommandComment::InOut: _res << "- *in,out*: "; break;
  }

  _res << "**" << (c_->isParamIndexValid()
    ? c_->getParamName(_fullComment)
    : c_->getParamNameAsWritten()).str() << "** ";

  visit(c_->getParagraph());
  _res << '\n';
}

void CommentToMarkdownConverter::visitTParamCommandComment(
  const TParamCommandComment* c_)
{
  _res << "- **" << c_->getParamNameAsWritten().str() << "**: ";
  visitNonStandaloneParagraphComment(c_->getParagraph());
  _res << '\n';
}

void CommentToMarkdownConverter::visitVerbatimBlockComment(
  const VerbatimBlockComment* c_)
{
  unsigned numLines = c_->getNumLines();

  if (!numLines)
    return;

  _res << "```\n";
  for (unsigned i = 0; i != numLines; ++i)
  {
    _res << c_->getText(i).str();
    if (i + 1 != numLines)
      _res << '\n';
  }
  _res << "\n```";
}

void CommentToMarkdownConverter::visitVerbatimLineComment(
  const VerbatimLineComment* c_)
{
  _res << '`' << c_->getText().str() << '`';
}

void CommentToMarkdownConverter::visitFullComment(const FullComment* c_)
{
  FullCommentParts parts(c_, _traits);

  bool _firstParagraphIsBrief = false;

  if (parts._headerfile)
    visit(parts._headerfile);

  if (parts._brief)
    visit(parts._brief);
  else if (parts._firstParagraph)
  {
    visitNonStandaloneParagraphComment(parts._firstParagraph);
    _firstParagraphIsBrief = true;
    _res << "\n\n";
  }

  for (unsigned i = 0, e = parts._miscBlocks.size(); i != e; ++i)
  {
    const Comment *C = parts._miscBlocks[i];

    if (_firstParagraphIsBrief && C == parts._firstParagraph)
      continue;

    visit(C);
    _res << "\n\n";
  }

  if (parts._tParams.size() != 0)
  {
    _res << "\n**Template parameters:**\n";

    for (unsigned i = 0, e = parts._tParams.size(); i != e; ++i)
      visit(parts._tParams[i]);
  }

  if (parts._params.size() != 0)
  {
    _res << "\n**Parameters:**\n";

    for (unsigned i = 0, e = parts._params.size(); i != e; ++i)
      visit(parts._params[i]);
  }

  if (parts._returns.size() != 0)
  {
    _res << "\n**Return:**\n";

    for (unsigned i = 0, e = parts._returns.size(); i != e; ++i)
      visit(parts._returns[i]);
  }
}

void CommentToMarkdownConverter::visitNonStandaloneParagraphComment(
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
  CommentToMarkdownConverter markdownConverter(
    fc_, ctx_.getCommentCommandTraits(), res);
  markdownConverter.visit(fc_);

  return res.str();
}

} // parser
} // cc
