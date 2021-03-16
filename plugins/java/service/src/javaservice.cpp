#include <service/javaservice.h>
#include <util/dbutil.h>
#include "../include/service/javaservice.h"


namespace cc
{
namespace service
{
namespace java
{

JavaServiceHandler::JavaServiceHandler(
  std::shared_ptr<odb::database> db_,
  std::shared_ptr<std::string> datadir_,
  const cc::webserver::ServerContext& context_)
    : _db(db_), _transaction(db_), _config(context_.options)
{
  new JavaProcess(*datadir_ + "/java", context_.compassRoot);
}

void java::JavaServiceHandler::getJavaString(std::string &str_)
{
  str_ = _config["java-result"].as<std::string>();
}

} // java
} // service
} // cc
