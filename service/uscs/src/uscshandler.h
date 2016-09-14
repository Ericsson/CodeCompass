#ifndef __CC_USCS_HANDLER_H__
#define __CC_USCS_HANDLER_H__

#include <uscs-api/UsageStatisticsService.h>

#include <odb/database.hxx>

#include "model/server.h"

namespace cc
{
namespace service
{
namespace stat
{

class UsageStatisticsServiceHandler : public UsageStatisticsServiceIf
{
public:
  /**
   * Creates a new UsageStatisticsServiceHandler.
   *
   * @param db_ a database instance.
   */
  UsageStatisticsServiceHandler(std::shared_ptr<odb::database> db_);

public:
  virtual void getServerId(
    std::string& _return,
    const ServerInfo& info_) override;
  
  virtual void addEntries(
    const std::string& serverId_,
    const std::vector<Entry>& entries_) override;

public:
  /**
   * Queries a server from the database by the given info. If the server does
   * not exists in the database it will be created. If it exists then it will
   * be updated with the give infos.
   *
   * This method must be called without an opened transaction. On any error an
   * exception will be thrown.
   *
   * @parma info_ info struct from a CC server via thift.
   * @return a Server database instance.
   */
  std::shared_ptr<Server> getCreateServerByInfo(ServerInfo info_);

  /**
   * Updates a server instance with the given informations.
   *
   * This method must be called without an opened transaction. On any error an
   * exception will be thrown.
   *
   * @parma info_ info struct from a CC server via thift.
   */
  void updateServerByInfo(Server& server_, const ServerInfo& info_);

public:
  /**
   * Sets the connected client's address for current address.
   *
   * @param addr_ address.
   */
  static void setCurrentClientAddress(const std::string& addr_);

private:
  /**
   * A database instance.
   */
  std::shared_ptr<odb::database> _db;
};

} // stat
} // service
} // cc

#endif // __CC_USCS_HANDLER_H__

