#include <memory>
#include <string>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Frontend/ASTConsumers.h>

#include <model/cppastnode-odb.hxx>

#include <util/logutil.h>

#include <cppparser/filelocutil.h>

#include "asthtml.h"
#include "astnodelocator.h"

namespace
{

using namespace cc;
using namespace cc::service::language;
using namespace llvm;
using namespace clang;

/**
 * A string wrapper that will escape HTML sequences as getFormatted() is called.
 */
class HTMLEscapingString : public UnformattedString
{
public:
  HTMLEscapingString() = default;
  explicit HTMLEscapingString(const char* ptr_) : UnformattedString(ptr_) {}
  explicit HTMLEscapingString(const std::string& str_)
    : UnformattedString(str_) {}
  ~HTMLEscapingString() override = default;

  std::string getFormatted() const override
  {
    bool monospaceEmitted = false;
    std::string buffer;
    buffer.reserve(static_cast<size_t>(_str.size() * 1.2));

    for(char c : _str)
    {
      switch (c)
      {
        // Escape HTML special characters.
        case '&':
          buffer.append("&amp;");
          break;
        case '\"':
          buffer.append("&quot;");
          break;
        case '\'':
          // Things between apostrophes are usually type or function names,
          // so we present them as monospace.
          if (!monospaceEmitted)
            buffer.append("<tt>");

          buffer.append("&apos;");

          if (monospaceEmitted)
            buffer.append("</tt>");

          monospaceEmitted = !monospaceEmitted;
          break;
        case '<':
          buffer.append("&lt;");
          break;
        case '>':
          buffer.append("&gt;");
          break;
        case ' ':
          buffer.append("&nbsp;");
          break;
        // Newlines should be newlines in the browser too.
        case '\n':
          buffer.append("<br />\n");
          break;
        default:
          buffer.append(&c, sizeof(char));
          break;
      }
    }

    return buffer;
  }
};

/**
 * A string wrapper that automatically expands into a colouring HTML <span>
 * tag when getFormatted() is called.
 */
class ColourTag : public UnformattedString
{
public:
  ColourTag() : _name(nullptr) {}

  ColourTag(raw_ostream::Colors colour_)
  {
    switch (colour_)
    {
      case raw_ostream::BLACK:
        _name = "black";
        break;
      case raw_ostream::RED:
        _name = "red";
        break;
      case raw_ostream::GREEN:
        _name = "green";
        break;
      case raw_ostream::YELLOW:
        _name = "yellow";
        break;
      case raw_ostream::BLUE:
        _name = "blue";
        break;
      case raw_ostream::MAGENTA:
        _name = "magenta";
        break;
      case raw_ostream::CYAN:
        _name = "cyan";
        break;
      case raw_ostream::WHITE:
        _name = "white";
        break;
      case raw_ostream::SAVEDCOLOR:
      default:
        _name = "";
        break;
    }
  }

  ~ColourTag() override = default;

  std::string getFormatted() const override
  {
    if (!_name)
      // Emit a closing tag if the colour tag constitutes the end of a
      // coloured block.
      return "</span>";
    else
      return std::string("<span class=\"ansi ") + _name + "\">";
  }

private:
  /**
   * The name of the colour to be used after this tag.
   */
  const char* _name;
};

/**
 * This class implements the functionality of Clang's AST-to-HTML printer in a
 * way that only a subtree corresponding to the user's requested node is
 * printed.
 */
class ASTSubtreeHTMLPrinter
  : public ASTConsumer,
    public RecursiveASTVisitor<ASTSubtreeHTMLPrinter>
{
  typedef RecursiveASTVisitor<ASTSubtreeHTMLPrinter> Base;

public:
  ASTSubtreeHTMLPrinter(std::unique_ptr<raw_ostream> out_,
                        ASTContext& context_,
                        model::CppAstNodePtr node_)
    : _out(std::move(out_)),
      _context(context_),
      _locator(context_, std::move(node_))
  {}

  void HandleTranslationUnit(ASTContext& context_) override
  {
    assert(&_context == &context_ && "AST visitor must be called with the same "
      "ASTContext as it was set up with.");
    TranslationUnitDecl* d = context_.getTranslationUnitDecl();
    TraverseDecl(d);
  }

  bool TraverseDecl(Decl* d_)
  {
    if (_locator.matchNodeAgainstLocation(d_))
    {
      d_->dump(*_out, /* Deserialize = */ false);
      return true;
    }
    return Base::TraverseDecl(d_);
  }

  bool TraverseStmt(Stmt* s_)
  {
    if (_locator.matchNodeAgainstLocation(s_))
    {
      s_->dump(*_out, _context);
      return true;
    }
    return Base::TraverseStmt(s_);
  }

private:

  std::unique_ptr<raw_ostream> _out;
  ASTContext& _context;
  cc::service::reparse::ASTNodeLocator _locator;
};

} // namespace (anonymous)


namespace cc
{

namespace service
{

namespace language
{

std::unique_ptr<clang::ASTConsumer> ASTHTMLActionFactory::newASTConsumer()
{
  assert(_stream && "Must not call newASTConsumer twice as the underlying "
    "stream has been moved out.");
  return clang::CreateASTDumper(std::move(_stream), "",
                                true, true, false, false, clang::ADOF_Default);
}

std::unique_ptr<clang::ASTConsumer>
ASTHTMLActionFactory::newASTConsumerForNode(clang::ASTContext& context_,
                                            model::CppAstNodePtr astNode_)
{
  assert(_stream && "Must not call newASTConsumer twice as the underlying "
    "stream has been moved out.");
  return std::make_unique<ASTSubtreeHTMLPrinter>(std::move(_stream),
                                                 context_, astNode_);
}

std::string ASTHTMLActionFactory::str() const
{
  return _out.str();
}

ColouredHTMLOutputStream::~ColouredHTMLOutputStream()
{
  flushToParts();

  // Format the string parts of the current stream into the wrapped string.
  for (UnformattedString* ptr : _parts)
  {
    _string << ptr->getFormatted();
    delete ptr;
  }
}

void ColouredHTMLOutputStream::flushToParts()
{
  flush();
  _lastPartAppendable = false;
}

void ColouredHTMLOutputStream::write_impl(const char* ptr_, size_t size_)
{
  if (_lastPartAppendable)
    _parts.back()->append(ptr_, size_);
  else
  {
    std::string str;
    str.append(ptr_, size_);

    UnformattedString* ptr = new HTMLEscapingString(str);
    _parts.push_back(ptr);
    _lastPartAppendable = true;
  }

  _bufferSize += size_;
}

ColouredHTMLOutputStream& ColouredHTMLOutputStream::changeColor(
  llvm::raw_ostream::Colors colour_, bool /*bold_*/, bool /*background_*/)
{
  // Finish writing everything that was in the buffer originally.
  flushToParts();

  UnformattedString* colourTag = new ColourTag(colour_);
  _parts.push_back(colourTag);

  return *this;
}

ColouredHTMLOutputStream& ColouredHTMLOutputStream::resetColor()
{
  // Finish writing everything that was in the buffer originally.
  flushToParts();

  UnformattedString* colourTag = new ColourTag();
  _parts.push_back(colourTag);

  return *this;
}

} //namespace language
} //namespace service
} //namespace cc
