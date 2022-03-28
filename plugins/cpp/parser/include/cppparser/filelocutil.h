#ifndef CC_PARSER_FILELOCUTIL_H
#define CC_PARSER_FILELOCUTIL_H

#include <string>

#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>

#include <model/position.h>

namespace cc
{
namespace parser
{

class FileLocUtil
{
public:
  FileLocUtil(const clang::SourceManager& clangSrcMan_)
    : _clangSrcMan(clangSrcMan_)
  {
  }

  /**
   * This function sets the range_ parameter based on the given location. The
   * range_ will begin at the beginning of loc_ and it will end at the end of
   * loc_. If loc_ is invalid then range_ stays untouched.
   */
  bool setRange(const clang::SourceLocation& loc_, model::Range& range_)
  {
    return setRange(loc_, loc_, range_);
  }

  /**
   * This function sets the range_ parameter based on the given start and end
   * locations. The range_ will begin at the beginning of start_ and it will end
   * at the end of end_. If either start_ or end_ is invalid then the function
   * returns false and range_ stays untouched.
   */
  bool setRange(
    const clang::SourceLocation& start_,
    const clang::SourceLocation& end_,
    model::Range& range_)
  {
    if (!setPosition(start_, range_.start) || !setPosition(end_, range_.end))
      return false;

    // For some reason usually getLocEnd() also returns the beginning of the
    // node. The real ending position of the node can be gotten by adding the
    // length of the token.
    clang::LangOptions langOpts;
    range_.end.column += clang::Lexer::MeasureTokenLength(
      end_, _clangSrcMan, langOpts);

    return true;
  }

  /**
   * This function sets position_ parameter to the location based on loc_.
   *
   * @param loc_ The source location of which we'd like to find the place.
   * @param position_ Position of the location.
   * @return If the given location is invalid then the function returns false.
   * In this case the position_ variable won't be set.
   */
  bool setPosition(
    const clang::SourceLocation& loc_,
    model::Position& position_)
  {
    if (loc_.isInvalid())
      return false;

    position_.line = _clangSrcMan.getPresumedLineNumber(loc_);
    position_.column = _clangSrcMan.getPresumedColumnNumber(loc_);

    return true;
  }

  /**
   * This function returns the absolute path of the file in which loc_ location
   * takes place. The location is meant to be the expanded location (in case of
   * macro expansion).
   * If the file can't be determined then an empty string is returned.
   */
  std::string getFilePath(const clang::SourceLocation& loc_)
  {
    return getFilePath(
      _clangSrcMan.getFileID(_clangSrcMan.getExpansionLoc(loc_)));
  }

  /**
   * This function returns the absolute path of the file identified by fid_.
   * If the file can't be determined then an empty string is returned.
   */
  std::string getFilePath(const clang::FileID& fid_)
  {
    if (fid_.isInvalid())
      return std::string();

    const clang::FileEntry* fileEntry = _clangSrcMan.getFileEntryForID(fid_);
    if (!fileEntry)
      return std::string();

    return fileEntry->tryGetRealPathName();
  }

private:
  const clang::SourceManager& _clangSrcMan;
};

} // namespace parser
} // namespace cc

#endif
