#ifndef USERSTAT_H
#define	USERSTAT_H

#include <string>
#include <cstdio>
#include <vector>
#include <mutex>
#include <memory>

#include <mongoose/userstatif.h>
#include <uscs-api/UsageStatisticsService.h>

// undef thrift config.h`s stuff
#undef PACKAGE
#undef PACKAGE_BUGREPORT
#undef PACKAGE_NAME
#undef PACKAGE_STRING
#undef PACKAGE_TARNAME
#undef PACKAGE_VERSION
#undef VERSION

#include <config.h>

#ifndef CC_USCS_DEFAULT_HOST
#define CC_USCS_DEFAULT_HOST ""
#endif

#ifndef CC_USCS_DEFAULT_PORT
#define CC_USCS_DEFAULT_PORT 0
#endif

namespace cc
{
namespace mongoose
{

class UserStat :
  public UserStatIf,
  public std::enable_shared_from_this<UserStat>
{
public:
  UserStat(
    const std::vector<std::string>& ports_,
    const std::string& filename_,
    const std::string& uscsHost_ = CC_USCS_DEFAULT_HOST,
    int uscsPort_ = CC_USCS_DEFAULT_PORT);
  
  ~UserStat();

public:
  virtual void logMethodCall(
    mg_connection* conn_,
    const std::string workspace_,
    const std::string handler_,
    const std::string method_) override;

  /**
   * Tries to send the content of the entry cache to the server. On any error
   * the current cache elements will be dropped.
   */
  void sendEntries();

private:
  std::unique_ptr<service::stat::UsageStatisticsServiceIf> connect();

  void writeEntry(const service::stat::Entry& entry_);

  bool isUSCSEnabled() const;
  
private:
  std::FILE* _logFile = nullptr;
  service::stat::ServerInfo _srvInfo;

  std::string _uscsHost;
  int _uscsPort;

  std::mutex _entryCacheMutex;
  std::vector<service::stat::Entry> _entryCache;
};

} // mongoose
} // cc

#endif
