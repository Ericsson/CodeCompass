#ifndef CC_SERVICE_JAVA_JAVASERVICE_H
#define CC_SERVICE_JAVA_JAVASERVICE_H

#include <memory>
#include <vector>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>
#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

#include <JavaService.h>

#include <service/javaprocess.h>

namespace cc
{
namespace service
{
namespace java
{

class JavaServiceHandler : virtual public JavaServiceIf
{
public:

  JavaServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> datadir_,
    const cc::webserver::ServerContext& context_);

  void getJavaString(std::string& str_);

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;

  const boost::program_options::variables_map& _config;
  std::unique_ptr<JavaProcess> _javaProcess;
};

} // java
} // service
} // cc

#endif // CC_SERVICE_JAVA_JAVASERVICE_H
