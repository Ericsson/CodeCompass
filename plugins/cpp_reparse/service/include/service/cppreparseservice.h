#ifndef CC_SERVICE_CPPREPARSESERVICE_H
#define CC_SERVICE_CPPREPARSESERVICE_H

#include <string>
#include <memory>

#include <boost/program_options/variables_map.hpp>

#include <odb/database.hxx>

#include <util/odbtransaction.h>
#include <webserver/servercontext.h>

#include <CppReparseService.h>

namespace cc
{

namespace service
{

namespace reparse
{

class ASTCache;
class CppReparser;

} // namespace reparse

namespace language
{

class CppReparseServiceHandler: virtual public CppReparseServiceIf
{
public:
  CppReparseServiceHandler(
    std::shared_ptr<odb::database> db_,
    std::shared_ptr<std::string> /* datadir_ */,
    const cc::webserver::ServerContext& context_);

  ~CppReparseServiceHandler();

  virtual bool isEnabled() override;

  virtual void getAsHTML(
    std::string& return_,
    const core::FileId& fileId_) override;

private:
  std::shared_ptr<odb::database> _db;
  util::OdbTransaction _transaction;
  const boost::program_options::variables_map& _config;

  std::shared_ptr<reparse::ASTCache> _astCache;
  std::unique_ptr<reparse::CppReparser> _reparser;
};

} // namespace language
} // namespace service
} // namespace cc

#endif // CC_SERVICE_CPPREPARSESERVICE_H
