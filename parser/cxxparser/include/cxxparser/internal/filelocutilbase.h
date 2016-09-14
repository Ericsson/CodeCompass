#ifndef CXXPARSER_INTERNAL_FILE_LOCATION_UTIL_BASE_H
#define CXXPARSER_INTERNAL_FILE_LOCATION_UTIL_BASE_H

#include <clang/Basic/SourceManager.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/LangOptions.h>
#include <clang/Lex/Lexer.h>

#include <model/position.h>

#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>

namespace cc
{
namespace parser
{
  
class FileLocUtilBase
{
public:
  FileLocUtilBase(const clang::SourceManager& clangSrcMan_) :
    _clangSrcMan(clangSrcMan_)
  {
  }

public:
  bool setPosition(const clang::SourceLocation& clangLoc_,
                   model::Range&                floc_)
  {
    return setPosition(clangLoc_, clangLoc_, floc_);
  }

  bool setPosition(const clang::SourceLocation& startLoc_,
                   const clang::SourceLocation& endLoc_,
                   model::Range&                floc_)
  {
    if (!getStartLineInfo(startLoc_, floc_.start.line, floc_.start.column) ||
        !getStartLineInfo(endLoc_, floc_.end.line, floc_.end.column))
    {
      return false;
    }

    // TODO: We should get the language options
    clang::LangOptions langOpts;
    floc_.end.column += clang::Lexer::MeasureTokenLength(endLoc_,
                                                     _clangSrcMan,
                                                     langOpts);

    return true;
  }

  bool getFilePath(const clang::SourceLocation& loc_,
                   std::string&                 filePath_)
  {
    if (loc_.isInvalid())
    {
      return false;
    }

    auto expLoc = _clangSrcMan.getExpansionLoc(loc_);
    clang::FileID fId = _clangSrcMan.getFileID(expLoc);
    if (fId.isInvalid())
    {
      return false;
    }

    const clang::FileEntry* fEntry = _clangSrcMan.getFileEntryForID(fId);
    if (!fEntry)
    {
      return false;
    }

    filePath_ = fEntry->getName();

    return true;
  }

//protected:
  bool getStartLineInfo(const clang::SourceLocation& loc_,
                        model::Position::postype&    line_,
                        model::Position::postype&     col_)
  {
    if (loc_.isInvalid())
    {
      return false;
    }

    //SLog() << "printToString: " << loc_.printToString(_clangSrcMan);

    auto expLoc = _clangSrcMan.getExpansionLoc(loc_);
    auto presLoc = _clangSrcMan.getPresumedLoc(expLoc);

    if (presLoc.isInvalid())
      return false;


    line_ = presLoc.getLine();
    col_ = presLoc.getColumn();

    return true;
  }

protected:
  void getBuffer(
    clang::SourceLocation loc_,
    const char*& buffer_,
    std::size_t& bufferSize_)
  {
    clang::FileID fid = _clangSrcMan.getFileID(loc_);
    if (fid.isInvalid())
    {
      return;
    }

    bool buffInvalid = false;
    llvm::StringRef buff = _clangSrcMan.getBufferData(fid, &buffInvalid);
    if (buffInvalid)
    {
      return;
    }

    buffer_ = buff.data();
    bufferSize_ = buff.size();
  }

protected:
  const clang::SourceManager& _clangSrcMan;
};

} // parser
} // cc

#endif // CXXPARSER_INTERNAL_FILE_LOCATION_UTIL_BASE_H
