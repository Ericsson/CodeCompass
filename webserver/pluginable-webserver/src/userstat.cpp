#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/types.h>
#include <sys/utsname.h>
#include <ifaddrs.h>

#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include <uscs-api/uscs_constants.h>
#include <util/streamlog.h>
#include <util/standarderrorlogstrategy.h>
#include "mongoose_utility.h"
#include "userstat.h"

namespace
{

/**
 * Returns the default interface.
 */
std::string getDefaultInterface()
{
  FILE* route;
  char line[1000] , *iface, *dest, *saveptr;
  
  route = ::fopen("/proc/net/route" , "r");
  while (::fgets(line , 1000 , route))
  {
    saveptr = nullptr;
    iface = ::strtok_r(line, " \t", &saveptr);
    dest = ::strtok_r(nullptr, " \t", &saveptr);
    
    if (iface != nullptr && dest != nullptr)
    {
      if (::strcmp(dest, "00000000") == 0)
      {
        return iface;
      }
    }
  }

  return "";
}

/**
 * @param addr_ the public addresses.
 * @param iface_ interface filter.
 */
void getAddresses(std::vector<std::string>& addr_, const std::string& iface_)
{
  struct ifaddrs* ifas = nullptr;

  if (::getifaddrs(&ifas) == -1 || !ifas)
  {
    SLog() << "Failed to get addresses!";
    return;
  }

  // Could be INET6_ADDRSTRLEN which is larger then INET_ADDRSTRLEN but who
  // cares
  char addrBuff[100];
  void* addrPtr;
  for (auto ifa = ifas; ifa; ifa = ifa->ifa_next)
  {
    if (std::strstr(ifa->ifa_name, iface_.c_str()) != ifa->ifa_name ||
        !ifa->ifa_addr)
    {
      // this is not an ethxx interface or ifa_addr is NULL, skip it
      continue;
    }
    else if (ifa->ifa_addr->sa_family == AF_INET)
    {
      // IPv4 address
      addrPtr=&((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
      inet_ntop(AF_INET, addrPtr, addrBuff, sizeof(addrBuff));

      if (strcmp(addrBuff, "127.0.0.1") != 0)
      {
        addr_.emplace_back(addrBuff);
      }
    }
    else if (ifa->ifa_addr->sa_family == AF_INET6)
    {
      // IPv6 address
      addrPtr=&((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;
      inet_ntop(AF_INET6, addrPtr, addrBuff, sizeof(addrBuff));

      if (strcmp(addrBuff, "::1") != 0)
      {
        addr_.emplace_back(addrBuff);
      }
    }
  }

  freeifaddrs(ifas);
}

} // anonymous namespace

namespace cc
{
namespace mongoose
{

UserStat::UserStat(
  const std::vector<std::string>& ports_,
  const std::string& filename_,
  const std::string& uscsHost_,
  int uscsPort_) :
  _logFile(nullptr),
  _uscsHost(uscsHost_),
  _uscsPort(uscsPort_)
{
  using namespace service::stat;

  if (!filename_.empty())
  {
    _logFile = std::fopen(filename_.c_str(), "a");
  }

  auto iface = getDefaultInterface();
  if (!iface.empty())
  {
    SLog() << "The default root interface is: " << iface;
    getAddresses(_srvInfo.addresses, iface);
  }

  _srvInfo.ports = ports_;
  _srvInfo.infos[g_uscs_constants.SRV_INFO_REV] = CC_GIT_VERSION;
 
  struct utsname unameb;
  std::memset(&unameb, 0, sizeof(unameb));
  if (uname(&unameb) == 0)
  {
    _srvInfo.infos[g_uscs_constants.SRV_INFO_UNAME_SYSNAME] = unameb.sysname;
    _srvInfo.infos[g_uscs_constants.SRV_INFO_UNAME_NODENAME] = unameb.nodename;
    _srvInfo.infos[g_uscs_constants.SRV_INFO_UNAME_RELEASE] = unameb.release;
    _srvInfo.infos[g_uscs_constants.SRV_INFO_UNAME_VERSION] = unameb.version;
    _srvInfo.infos[g_uscs_constants.SRV_INFO_UNAME_MACHINE] = unameb.machine;
    _srvInfo.infos[g_uscs_constants.SRV_INFO_UNAME_DOMAIN] = unameb.domainname;
  } 
}

UserStat::~UserStat()
{
  if (_logFile)
  {
    std::fclose(_logFile);
  }
}

void UserStat::logMethodCall(
  mg_connection* conn_,
  const std::string workspace_,
  const std::string handler_,
  const std::string method_)
{
  using namespace service::stat;

  service::stat::Entry ent;
  ent.timestamp = std::time(nullptr);
  ent.data[g_uscs_constants.ENTRY_USER] = getAuthToken(conn_);
  ent.data[g_uscs_constants.ENTRY_WORKSPACE] = workspace_;
  ent.data[g_uscs_constants.ENTRY_SERVICE] = handler_;
  ent.data[g_uscs_constants.ENTRY_METHOD] = method_;

  writeEntry(ent);

  if (isUSCSEnabled())
  {
    std::lock_guard<std::mutex> lock(_entryCacheMutex);
    _entryCache.emplace_back(std::move(ent));
  }
}

void UserStat::sendEntries()
{
  if (!isUSCSEnabled())
  {
    return;
  }

  // Get and clear cache
  std::vector<service::stat::Entry> items;
  {
    std::lock_guard<std::mutex> lock(_entryCacheMutex);
    _entryCache.swap(items);
  }

  try
  {
    auto service = connect();
    std::string serverId;
    service->getServerId(serverId, _srvInfo);
    service->addEntries(serverId, items);
  }
  catch (const std::exception& ex)
  {
    SLog() << "USCS connection error: " << ex.what();
  }
  catch (...)
  {
    // Supress any exception
    SLog() << "USCS connection error!";
  }
}

std::unique_ptr<service::stat::UsageStatisticsServiceIf> UserStat::connect()
{
  using namespace service::stat;
  using namespace apache::thrift::transport;
  using namespace apache::thrift::protocol;

  boost::shared_ptr<TSocket> conn(new TSocket(_uscsHost, _uscsPort));
  conn->open();
  boost::shared_ptr<TProtocol> proto(new TBinaryProtocol(conn));

  return std::unique_ptr<service::stat::UsageStatisticsServiceIf>(
    new service::stat::UsageStatisticsServiceClient(proto));
}

void UserStat::writeEntry(const service::stat::Entry& entry_)
{
  if (!_logFile)
  {
    return;
  }

  std::stringstream out;

  char buffer[20];
#ifdef __i386
  //surely needs conversion on x86.
  //conversion is a bad idea if sizeof(time_t) > sizeof(timestamp)
  static_assert(sizeof(time_t) <= sizeof(entry_.timestamp), "time_t error");
  std::strftime(buffer, 20, "%F %T", std::localtime((const time_t*)&entry_.timestamp));
#else
  std::strftime(buffer, 20, "%F %T", std::localtime(&entry_.timestamp));
#endif

  //out << std::put_time(&tm, "%F %T"); FIXME: its not supported in gcc 4.9
  out << buffer;
  for (const auto& value : entry_.data)
  {
    out << " " << value.first << "=" << value.second;
  }

  std::fprintf(_logFile, "%s\n", out.str().c_str());
  std::fflush(_logFile);
}

bool UserStat::isUSCSEnabled() const
{
  if (_uscsHost.empty() || _uscsPort <= 0)
  {
    return false;
  }

  return true;
}

} // mongoose
} // cc

