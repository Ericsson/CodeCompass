#include <ctime>
#include <util/util.h>

namespace cc
{
namespace util
{

std::string getCurrentDate()
{
  std::time_t rawtime;
  struct tm* timeinfo;

  std::time(&rawtime);
  timeinfo = std::localtime(&rawtime);

  char output[42];
  std::strftime(output, 42, "%c", timeinfo);

  return {output};
}

}
}
