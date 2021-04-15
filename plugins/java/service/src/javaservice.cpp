#include <service/javaservice.h>
#include <util/dbutil.h>


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
  _javaProcess.reset(new JavaServiceProcess(*datadir_ + "/java",
                                            context_.compassRoot));
}

void JavaServiceHandler::getJavaString(std::string &str_)
{
  str_ = _config["java-result"].as<std::string>();
}

} // java
} // service
} // cc
