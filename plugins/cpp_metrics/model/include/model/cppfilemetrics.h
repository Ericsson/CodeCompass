#ifndef CC_MODEL_CPPFILEMETRICS_H
#define CC_MODEL_CPPFILEMETRICS_H

#include <model/file.h>

namespace cc
{
namespace model
{

#pragma db object
struct CppFileMetrics
{
  enum Type
  {
    EFFERENT_MODULE,
    RELATIONAL_COHESION_MODULE
  };

  #pragma db id auto
  std::uint64_t id;

  #pragma db not_null
  FileId file;

  #pragma db not_null
  Type type;

  #pragma db not_null
  double value;
};

#pragma db view \
  object(CppFileMetrics) \
  object(File : CppFileMetrics::file == File::id)
struct CppModuleMetricsForPathView
{
  #pragma db column(CppFileMetrics::file)
  FileId fileId;

  #pragma db column(File::path)
  std::string path;

  #pragma db column(CppFileMetrics::type)
  CppFileMetrics::Type type;

  #pragma db column(CppFileMetrics::value)
  double value;
};

#pragma db view \
  object(CppFileMetrics) \
  object(File : CppFileMetrics::file == File::id) \
  query(distinct)
struct CppModuleMetricsDistinctView
{
  #pragma db column(CppFileMetrics::file)
  FileId fileId;
};

} //model
} //cc

#endif //CC_MODEL_CPPFILEMETRICS_H
