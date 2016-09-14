#ifndef UTIL_SOURCE_TEXT_RANGE_H
#define UTIL_SOURCE_TEXT_RANGE_H

namespace cc 
{
namespace util 
{

/**
 * Represents a text range in a source file.
 */
class SourceTextRange
{
public:
  /**
   * Typedef for line and column positions.
   */
  typedef std::size_t pos_t;

public:
  /**
   * A constructor.
   */
  SourceTextRange() :
    _startLine(0),
    _startColumn(0),
    _endLine(0),
    _endColumn(0)
  {
  }

  /**
   * A constructor.
   */
  SourceTextRange(pos_t    startLine_,
                  pos_t    startColumn_,
                  pos_t    endLine_,
                  pos_t    endColumn_) :
    _startLine(startLine_),
    _startColumn(startColumn_),
    _endLine(endLine_),
    _endColumn(endColumn_)
  {
  }

public:
  /**
   * Returns whether the text range is valid or not.
   *
   * @return true if the range is valid, false otherwise.
   */
  bool isValid() const
  {
    return _startLine > 0     &&
           _startColumn > 0   &&
           _endLine > 0       &&
           _endColumn > 0     &&
           _startLine <= _endLine;
  }

  /**
    * Returns whether the current range fully contains the range given.
    * @param other_ The range which is or not contained by the current range.
    * @return True if the current range contains other_, false otherwise.
    */
  bool contains(const SourceTextRange &other_)
  {
    if (getStartLine() > other_.getStartLine() || getEndLine() < other_.getEndLine())
      return false;

    if (getStartLine() == other_.getStartLine() && getEndLine() == other_.getEndLine())
      return getStartColumn() <= other_.getStartColumn() && getEndColumn() >= other_.getEndColumn();
    else if (getStartLine() == other_.getStartLine())
      return getStartColumn() <= other_.getStartColumn();
    else
      return getEndColumn() >= other_.getEndColumn();

    // should never be executed because above code covers all cases
    return false;
  }

public:
  /**
   * Getter method for the start line.
   */
  pos_t getStartLine() const
  {
    return _startLine;
  }

  /**
   * Getter method for the start column.
   */
  pos_t getStartColumn() const
  {
    return _startColumn;
  }

  /**
   * Getter method for the end line.
   */
  pos_t getEndLine() const
  {
    return _endLine;
  }

  /**
   * Getter method for the start column.
   */
  pos_t getEndColumn() const
  {
    return _endColumn;
  }

  /**
   * Setter method for start position.
   */
  void setStart(pos_t startLine_, pos_t startColumn_)
  {
    _startLine = startLine_;
    _startColumn = startColumn_;
  }

  /**
   * Setter method for end position.
   */
  void setEnd(pos_t endLine_, pos_t endColumn_)
  {
    _endLine = endLine_;
    _endColumn = endColumn_;
  }

private:
  /**
   * Start line.
   */
  pos_t _startLine;

  /**
   * Start column.
   */
  pos_t _startColumn;

  /**
   * End line.
   */
  pos_t _endLine;

  /**
   * End column.
   */
  pos_t _endColumn;
};

} // util
} // cc

#endif // UTIL_SOURCE_TEXT_RANGE_H
