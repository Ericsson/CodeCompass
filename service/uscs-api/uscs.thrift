namespace cpp cc.service.stat

const string SRV_INFO_REV = "REV"
const string SRV_INFO_UNAME_SYSNAME = "UNAME_SYSNAME"
const string SRV_INFO_UNAME_NODENAME = "UNAME_NODENAME"
const string SRV_INFO_UNAME_RELEASE = "UNAME_RELEASE"
const string SRV_INFO_UNAME_VERSION = "UNAME_VERSION"
const string SRV_INFO_UNAME_MACHINE = "UNAME_MACHINE"
const string SRV_INFO_UNAME_DOMAIN = "UNAME_DOMAIN"

const string ENTRY_USER = "USER"
const string ENTRY_WORKSPACE = "WS"
const string ENTRY_SERVICE = "SERVICE"
const string ENTRY_METHOD = "METHOD"

/**
 * Generic exception for the statistics server.
 */
exception ServiceError
{
  1:string message
}

/**
 * Basic server info.
 */
struct ServerInfo
{
  /**
   * Normalized server addresses. An address is an IPv4/IPv6 address (not the
   * hostname).
   */
  1:list<string> addresses,
  /**
   * Server ports.
   */
  2:list<string> ports,
  /**
   * Other infos in key-value form. (See SRV_INFO_* constants)
   */
  3:map<string, string> infos
}

/**
 * A statistics entry.
 */
struct Entry
{
  /**
   * Entry timestamp.
   */
  1:i64 timestamp,
  /**
   * Key-value pairs. (See ENTRY_* constants)
   */
  2:map<string, string> data
}

/**
 * The USCS (Usage Statistics Collector Server) interface.
 */
service UsageStatisticsService
{
  /**
   * Returns the server id for the given server.
   *
   * @param info the server info for identification.
   * @return the server id.
   * @throws ServerError on any error
   */
  string getServerId(1: ServerInfo info) throws (1:ServiceError ex),

  /**
   * Adds new entries to a server statistics.
   *
   * @param serverId a server id (see getServerData)
   * @param enrties the new entries
   */
  void addEntries(1:string serverId, 2: list<Entry> entries)
}
