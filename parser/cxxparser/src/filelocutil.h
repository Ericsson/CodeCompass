#ifndef CXXPARSER_FILE_LOCATION_UTIL_H
#define CXXPARSER_FILE_LOCATION_UTIL_H

#include <parser/sourcemanager.h>

#include <model/fileloc.h>
#include <cxxparser/internal/filelocutilbase.h>

namespace cc
{
namespace parser
{
  
class FileLocUtil : public FileLocUtilBase
{
public:
  FileLocUtil(SourceManager&              srcMan_,
              const clang::SourceManager& clangSrcMan_);

  bool setInfo(const clang::SourceLocation& clangLoc_,
               model::FileLoc&              floc_);

  bool setInfo(const clang::SourceLocation& startLoc_,
               const clang::SourceLocation& endLoc_,
               model::FileLoc&              floc_);

private:
  bool setFile(const clang::SourceLocation& loc_,
               model::FileLoc&              floc_);

private:
  SourceManager& _srcMan;
};

} // parser
} // cc

#endif // CXXPARSER_FILE_LOCATION_UTIL_H
