#include "uscshandler.h"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "model/server-odb.hxx"
#include "model/entry-odb.hxx"

namespace
{

thread_local std::vector<std::string> clientAddresses;

/**
 * Helper function for updateDiffs().
 */
template <typename ObjT, typename QueryT>
std::shared_ptr<ObjT> createGet(
  odb::database& db_,
  const ObjT& obj_,
  const QueryT& query_)
{
  std::shared_ptr<ObjT> objPtr = std::make_shared<ObjT>(obj_);

  try
  { 
    odb::transaction trans(db_.begin());
      db_.persist(objPtr);
    trans.commit();
  }
  catch (const odb::exception& ex)
  {
    odb::transaction trans(db_.begin());

    // Try to get the object
    auto res = db_.query<ObjT>(query_);
    auto it = res.begin();
    if (it == res.end())
    {
      throw;
    }

    *objPtr = *it;
  }

  return objPtr;
}

/**
 * Helper function for updating the addresses or ports vector of a Server.
 */
template <typename ObjT, typename QueryColT>
void updateDiffs(
  odb::database& db_,
  odb::vector<std::shared_ptr<ObjT>>& current_,
  const std::vector<std::string>& new_,
  std::string ObjT::*objKey_,
  const QueryColT& queryCol_)
{
  std::vector<std::string> toAdd = new_;

  auto it = current_.begin();
  while (it != current_.end())
  {
    auto toAddIt = std::find(toAdd.begin(), toAdd.end(), (**it).*objKey_);
    if (toAddIt != toAdd.end())
    {
      // It's not a new item (already added to the server)
      toAdd.erase(toAddIt);
      ++it;
    }
    else
    {
      // This address is removed from this server
      it = current_.erase(it);
    }
  }

  ObjT newObj;
  odb::query<ObjT> query(queryCol_ == odb::query<ObjT>::_ref(newObj.*objKey_));
  for (const std::string& val : toAdd)
  {
    newObj.*objKey_ = val;
    current_.emplace_back(createGet(db_, newObj, query));
  }
}

bool tryGetIpsFromHostname(
  const std::string& hostname_,
  std::vector<std::string>& addrs_)
{
  struct addrinfo hints;
  struct addrinfo* res = nullptr;
  int status;
  
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
 
  if ((status = ::getaddrinfo(hostname_.c_str(), nullptr, &hints, &res)) != 0)
  {
    return false;
  }

  // Could be INET6_ADDRSTRLEN which is larger then INET_ADDRSTRLEN but who
  // cares
  char addrBuff[100];
  void* addrPtr;
  for (struct addrinfo* it = res; it != nullptr; it = it->ai_next)
  {
    if (it->ai_family == AF_INET)
    {
      // IPv4 address
      addrPtr=&((struct sockaddr_in*)it->ai_addr)->sin_addr;
      ::inet_ntop(AF_INET, addrPtr, addrBuff, sizeof(addrBuff));
    }
    else if (it->ai_family == AF_INET6)
    {
      // IPv6 address
      addrPtr=&((struct sockaddr_in6*)it->ai_addr)->sin6_addr;
      ::inet_ntop(AF_INET6, addrPtr, addrBuff, sizeof(addrBuff));
    }
    else
    {
      continue;
    }

    addrs_.emplace_back(addrBuff);
  }

  ::freeaddrinfo(res);
  return !addrs_.empty();
}

} // anonymous namespace

namespace cc
{
namespace service
{
namespace stat
{

UsageStatisticsServiceHandler::UsageStatisticsServiceHandler(
  std::shared_ptr<odb::database> db_) :
  _db(db_)
{
}

void UsageStatisticsServiceHandler::getServerId(
  std::string& _return,
  const ServerInfo& info_)
{
  try
  {
    try
    {
      auto server = getCreateServerByInfo(info_);
      _return = std::to_string(server->id);
    }
    catch (const odb::exception& ex)
    {
      ServiceError err;
      err.message  = "Database error: ";
      err.message += ex.what();
      throw err;
    }
    catch (const ServiceError&)
    {
      throw;
    }
    catch (...)
    {
      ServiceError err;
      err.message = "Unknown error!";
      throw err;
    }
  }
  catch (const ServiceError& ex)
  {
    std::cerr << "getServerId failed: " << ex.message << std::endl;
    throw;
  }
}
 
void UsageStatisticsServiceHandler::addEntries(
  const std::string& serverId_,
  const std::vector<Entry>& entries_)
{
  if (entries_.empty())
  {
    return;
  }

  try
  {
    try
    {
      UsageEntry dbEntry;
      {
        odb::transaction trans(_db->begin());
        dbEntry.server = _db->load<Server>(std::stoull(serverId_));
      }
    
      for (const Entry& entry : entries_)
      {
        dbEntry.timestamp = entry.timestamp;
        dbEntry.props = entry.data;
    
        odb::transaction trans(_db->begin());
          _db->persist(dbEntry);
        trans.commit();
      }
    }
    catch (const odb::exception& ex)
    {
      ServiceError err;
      err.message  = "Database error: ";
      err.message += ex.what();
      throw err;
    }
    catch (...)
    {
      ServiceError err;
      err.message = "Unknown error!";
      throw err;
    }
  }
  catch (const ServiceError& ex)
  {
    std::cerr
      << "addEntries for " << serverId_ << "failed: "
      << ex.message << std::endl;
    throw;
  }
}

std::shared_ptr<Server> UsageStatisticsServiceHandler::getCreateServerByInfo(
  ServerInfo info_)
{
  using Query = odb::query<ServerByInfo>;

  // Filter out empty addresses
  {
    auto it = info_.addresses.begin();
    while (it != info_.addresses.end())
    {
      if (it->empty())
      {
        it = info_.addresses.erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  if (info_.addresses.empty())
  {
    if (clientAddresses.empty())
    {
      ServiceError err;
      err.message = "Missing address!";
      throw err;
    }
    else
    {
      std::cerr << "Using fallback addresses: " << std::endl;
      for (const std::string& addr : clientAddresses)
      {
        std::cerr << addr << std::endl;
      }
      std::cerr << std::endl;

      info_.addresses = clientAddresses;
    }
  }
  
  if (info_.ports.empty())
  {
    ServiceError err;
    err.message = "Missing port!";
    throw err;
  }

  std::shared_ptr<Server> server;
  {
    odb::transaction trans(_db->begin());
    
    auto res = _db->query<ServerByInfo>(
      Query::ServerAddress::address.in_range(
        info_.addresses.begin(), info_.addresses.end()) &&
      Query::ServerPort::port.in_range(
        info_.ports.begin(), info_.ports.end()));

    auto it = res.begin();
    if (it != res.end())
    {
      server = _db->load<Server>(it->id);
    }
    else
    {
      server = std::make_shared<Server>();
      _db->persist(server);
    }

    trans.commit();
  }

  updateServerByInfo(*server, info_);
  return server;
}

void UsageStatisticsServiceHandler::updateServerByInfo(Server& server_,
  const ServerInfo& info_)
{
  updateDiffs(*_db, server_.addresses, info_.addresses,
    &ServerAddress::address, odb::query<ServerAddress>::address);
  updateDiffs(*_db, server_.ports, info_.ports,
    &ServerPort::port, odb::query<ServerPort>::port);
  server_.props = info_.infos;
  server_.lastContact = std::time(nullptr);

  {
    odb::transaction trans(_db->begin());
      _db->update(server_);
    trans.commit();
  }
}

void UsageStatisticsServiceHandler::setCurrentClientAddress(
  const std::string& addr_)
{
  if (!tryGetIpsFromHostname(addr_, clientAddresses))
  {
    std::cerr << "Address lookup failed for " << addr_ << std::endl;
  }
}

} // stat
} // service
} // cc

