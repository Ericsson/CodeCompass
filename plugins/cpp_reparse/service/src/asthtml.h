#ifndef CC_SERVICE_CPPREPARSESERVICE_ASTHTML_H
#define CC_SERVICE_CPPREPARSESERVICE_ASTHTML_H

#include <memory>
#include <sstream>

#include <llvm/Support/raw_ostream.h>

#include <clang/AST/ASTConsumer.h>

#include <model/cppastnode.h>

namespace cc
{

namespace service
{

namespace language
{

/**
 * Wraps a string into a type that can define optional formatting methods in
 * the subtypes. The base class does not implement any special formatting
 * and will return the string contained within verbatim.
 */
class UnformattedString
{
public:
  UnformattedString() = default;
  explicit UnformattedString(const char* ptr_) : _str(ptr_) {}
  explicit UnformattedString(const std::string& str_) : _str(str_) {}
  virtual ~UnformattedString() = default;

  UnformattedString& append(const char* ptr_, size_t size_)
  {
    _str.append(ptr_, size_);
    return *this;
  }

  /**
   * @return The string contained by the wrapper with the particular type's
   * formatting applied to it.
   */
  virtual std::string getFormatted() const { return _str; }

protected:
  std::string _str;
};

/**
 * Provides an llvm::raw_ostream implementation that can format colourful
 * sentences into HTML colours.
 */
class ColouredHTMLOutputStream : public llvm::raw_ostream
{
public:
  ColouredHTMLOutputStream(std::ostringstream& os_)
    : raw_ostream(false),
      _string(os_),
      _lastPartAppendable(false),
      _bufferSize(0)
  {}

  ~ColouredHTMLOutputStream() override;

  bool is_displayed() const override { return true; }
  bool has_colors() const override { return true; }

  /**
   * Change the foreground color of the text that will be output from this
   * point forward.
   *
   * @param colour_ ANSI colour to use.
   * The boldness and background attributes are ignored by this implementation.
   */
  ColouredHTMLOutputStream& changeColor(Colors colour_,
                                        bool /*bold_*/,
                                        bool /*background_*/) override;

  ColouredHTMLOutputStream& resetColor() override;

private:
  std::ostringstream& _string;
  std::vector<UnformattedString*> _parts;

  /**
   * Indicates if the last element of _parts is something that write_impl
   * should append to if subsequent write_impl calls are made.
   */
  bool _lastPartAppendable;

  size_t _bufferSize;

  /**
   * Calls flush() on the inherited buffer and handles keeping the state of
   * the current class' members.
   */
  void flushToParts();

  /**
   * Return the current position within the stream, not counting the bytes
   * currently in the buffer.
   */
  uint64_t current_pos() const override { return _bufferSize; }

  /**
   * When LLVM decides that the buffer has overflown a certain limit, this
   * method is called to perform the actual writing into the backend. This
   * method should not assume anything about the current contents that are
   * to be written.
   */
  void write_impl(const char* ptr_, size_t size_) override;
};

/**
 * This class is to be used by clang::tooling::newFrontendActionFactory to
 * create a FrontendAction that runs the HTML-generating AST consumer on the
 * files passed to it.
 */
class ASTHTMLActionFactory
{
public:
  ASTHTMLActionFactory()
    : _stream(std::make_unique<ColouredHTMLOutputStream>(_out))
  {}

  std::unique_ptr<clang::ASTConsumer> newASTConsumer();

  /**
   * Retrieve the HTML string that was populated by the ASTConsumer created by
   * this factory.
   */
  std::string str() const;

private:
  /**
   * The string in which the output is collected.
   */
  std::ostringstream _out;

  /**
   * An LLVM output stream that wraps over _out, allowing the ASTConsumer to
   * put its output into the string.
   */
  std::unique_ptr<llvm::raw_ostream> _stream;
};

} //namespace language
} //namespace service
} //namespace cc

#endif // CC_SERVICE_CPPREPARSESERVICE_ASTHTML_H
