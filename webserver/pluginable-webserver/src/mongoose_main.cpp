//============================================================================
// Name        : PluggableMongoose.cpp
// Author      : Zoltan Borok-Nagy
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <iostream>
#include <algorithm>
#include <streambuf>
#include <memory>
#include <vector>
#include <exception>
#include <iomanip>
#include <ctime>
#include <thread>
#include <chrono>
#include <condition_variable>

#include <signal.h>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <odb/database.hxx>

#include <model/option.h>
#include <model/option-odb.hxx>

#include "util/util.h"
#include "util/logutil.h"
#include "util/streamlog.h"
#include "util/standarderrorlogstrategy.h"
#include "util/environment.h"
#include "util/odbtransaction.h"
#include "mongoose.h"
#include "mongoose/plugin.h"
#include "plugin/pluginhandler.h"

#include "mongoose_utility.h"
#include "authentication.h"
#include "threadedmongoose.h"
#include "userstat.h"

using namespace cc::plugin;
using namespace cc::mongoose;

namespace
{

class UserStatSender
{
public:
  UserStatSender(std::shared_ptr<UserStat> stat_) :
    _userStat(stat_),
    _userStatThread(&UserStatSender::senderThread, this)
  {
  }

  ~UserStatSender()
  {
    _senderEnabled = false;
    _stopSender.notify_one();
    _userStatThread.join();
  }

private:
  void senderThread()
  {
    auto dur = std::chrono::hours(1);
    while (_senderEnabled)
    {
      std::unique_lock<std::mutex> lock(_stopSenderMutex);
      if (_stopSender.wait_for(lock, dur, [this](){ return !_senderEnabled; }))
      {
        // _senderEnabled is false now -> exit thread.
        return;
      }
      else
      {
        // Variable timeout! Send statistics.
        _userStat->sendEntries();
      }
    }
  }

private:
  std::shared_ptr<UserStat> _userStat;
  std::thread _userStatThread;
  std::condition_variable _stopSender;
  std::mutex _stopSenderMutex;
  bool _senderEnabled = true;
};

struct MainRequestHandler
{
  enum class AuthMode { NONE, DIGEST, COOKIE, LDAP };

  PluginHandler<RequestHandler>      pluginHandler = RequestHandler::version;
  std::string                        documentRoot;
  std::string                        digestPasswdFile;
  AuthMode                           authMode = AuthMode::NONE;
  std::shared_ptr<UserStat>          userStat; 
  std::shared_ptr<UserStatSender>    userStatSender;
  std::shared_ptr<Authentication>    auth;
  std::map<std::string, std::string> dataDir;

  int begin_request_handler(struct mg_connection *conn)
  {
    //--- If not authenticated yet ---//  
    
    std::pair<std::string, std::string> userData;
    
    if (authMode == AuthMode::LDAP &&
      !auth->isAuthenticated(getAuthToken(conn)))
    {
      userData = getUsernamePassword(conn);
      std::string authtoken;
      
      if (!userData.first.empty())
        authtoken = auth->authenticate(userData.first, userData.second);
      
      if (authtoken.empty())
      {
        std::string loginPage = getLoginPage(documentRoot);
  
        mg_send_header(conn, "Content-Type", "text/html");
        mg_send_data(conn, loginPage.c_str(), loginPage.size());
  
        return MG_TRUE;
      }
      else
      {
        mg_send_header(conn, "Set-Cookie", (std::string("authtoken=") + authtoken).c_str());
      }
    }
    else if (auth && !auth->isAuthenticated(getAuthToken(conn)) && conn->content_len != 0)
    {
      mg_send_header(conn, "Set-Cookie",
        (std::string("authtoken=") + auth->authenticate(getRemoteIp(conn), "")).c_str());
    }
    
    //--- If already authenticated ---//
    
    std::string uri = conn->uri + 1; // We advance it by one because of
                                     // the '/' character
  
    SLog(cc::util::INFO)
      << getCurrentDate() << " Connection from " << conn->remote_ip
      << ':' << conn->remote_port << " requested URI: " << uri
      << std::endl;
  
    auto handler = pluginHandler.getImplementation(uri);
    if (handler)
    {
      return handler->beginRequest(conn, std::static_pointer_cast<UserStatIf>(
        userStat));
    }
    
    if (uri.find_first_of("doxygen/") == 0)
    {
      mg_send_file(conn, getDocDirByURI(uri).c_str());
      return MG_MORE;
    }
    
    // Returning MG_FALSE tells mongoose that we didn't served the request
    // so mongoose should serve it
    return MG_FALSE;
  }

  int operator()(struct mg_connection *conn, enum mg_event ev)
  {
    int result;

    switch (ev)
    {
      case MG_REQUEST:
        return begin_request_handler(conn);
        
      case MG_AUTH:
      {
        if (digestPasswdFile.empty())
          return MG_TRUE;
        
        FILE* fp = fopen(digestPasswdFile.c_str(), "r");
        if (fp) {
          result = mg_authorize_digest(conn, fp);
          fclose(fp);
          return result;
        } else {
          SLog(cc::util::ERROR)
            << "Password file could not be opened: " << digestPasswdFile;
          //throw an exception instead of a segfault/reauth
          //an internal server error response would be nicer
          throw std::runtime_error("Password file could not be opened.");
        }
      }

      default:
        break;
    }
    
    return MG_FALSE;
  }
  
private:
  std::string getDocDirByURI(std::string uri)
  {
    if (uri.empty())
      return "";
    
    if (uri.back() == '/')
      uri.pop_back();

    std::size_t pos1 = uri.find('/');
    std::size_t pos2 = uri.find('/', pos1 + 1);

    std::string ws = uri.substr(pos1 + 1, pos2 - pos1 - 1);
    
    std::string file;
    if (pos2 != std::string::npos)
      file = uri.substr(pos2);
    
    return dataDir[ws] + "/docs" + file;
  }
};

} // Unnamed namespace

std::string getDataDirFromDb(const std::string& connectionString)
{
  std::shared_ptr<odb::database> db
    = cc::util::createDatabase(connectionString);

  if (!db)
  {
    SLog(cc::util::ERROR)
      << "Wrong connection string: '" << connectionString << "' "
      << "for parser: 'doctoolparser'";

    throw std::runtime_error("Wrong database!");
  }

  return cc::util::OdbTransaction(db)([&db]() -> std::string {
    odb::result<cc::model::Option> options
      = db->query<cc::model::Option>(odb::query<cc::model::Option>());
    
    for (const cc::model::Option& option : options)
      if (option.key == "projectDataDir")
        return option.value;

    return "";
  });
}

int main(int argc, char* argv[])
{
  namespace po = ::boost::program_options;
  namespace fs = ::boost::filesystem;

  try
  {
    //--- Initialisation of logger ---//
    
    cc::util::StreamLog::initialize("CodeCompass", "Webserver");

    cc::util::StreamLog::setStrategy(
      std::shared_ptr<cc::util::LogStrategy>(
        new cc::util::StandardErrorLogStrategy()));

    cc::util::Environment::init();

    //--- Program options ---//
    
    po::options_description options("Options");
    po::options_description mongooseOptions = getMongooseOptions();
    po::options_description authOptions = getAuthOptions();

    options.add(getCoreOptions());
    options.add(mongooseOptions);
    options.add(authOptions);

    po::variables_map varMap;

    // At first we permit unknown options
    parseConfiguration(options, argc, argv, varMap, true);

    MainRequestHandler requestHandler;
    
    if (varMap.count(WORKSPACE_OPTION_NAME))
    {
      const auto& wses = varMap[WORKSPACE_OPTION_NAME].
        as<cc::mongoose::WorkspaceOptions>();

      std::cout << std::endl;
      for (const auto& ws : wses)
      {
        std::cout << "Workspace" << std::endl;
        std::cout << "  id = " << ws.workspaceId << std::endl;
        std::cout << "  connection = " << ws.connectionString << std::endl;
        std::cout << "  description = " << ws.description << std::endl;
        
        std::string dataDir
          = ws.dataDir.empty()
          ? getDataDirFromDb(ws.connectionString)
          : ws.dataDir;
        
        if (!dataDir.empty())
        {
          std::cout << "  datadir = " << dataDir << std::endl;
          requestHandler.dataDir[ws.workspaceId] = dataDir;
        }
        
        if (!ws.searchDir.empty())
          std::cout << "  searchdir = " << ws.searchDir << std::endl;
        
        std::cout << std::endl;
      }
    }

    //--- Authentication ---//
    
    std::shared_ptr<Persister> persister(
      new TextFileDatabase(varMap["auth_file"].as<std::string>().c_str()));
    
    if (varMap.count("auth_mode"))
    {
      std::string authModeStr = varMap["auth_mode"].as<std::string>();
      
      // We store authentication mode in an enumerated variable because this is checked every time
      // when a server request is accomplished and this is faster than string comparisons.
      if (authModeStr == "digest")
        requestHandler.authMode = MainRequestHandler::AuthMode::DIGEST;
      else if (authModeStr == "ldap")
        requestHandler.authMode = MainRequestHandler::AuthMode::LDAP;
      else
        requestHandler.authMode = MainRequestHandler::AuthMode::NONE;
      
      if (authModeStr == "ldap" && (!varMap.count("ldap_url") || !varMap.count("ldap_dn")))
        throw std::logic_error("ldap_url and ldap_dn must be given in case of LDAP authentication");
      
      if (authModeStr == "digest" && !varMap.count("global_auth_file"))
        throw std::logic_error("global_auth_file must be given in case of digest authentication");
        
      if (requestHandler.authMode == MainRequestHandler::AuthMode::LDAP)
      {
        requestHandler.auth.reset(new LdapAuthentication(
          varMap["ldap_url"].as<std::string>(),
          varMap["ldap_dn"].as<std::string>(),
          persister));
        
        if (varMap.count("ldap_cert"))
        {
          static_cast<LdapAuthentication*>(requestHandler.auth.get())->setCertPath(
            varMap["ldap_cert"].as<std::string>());
        }
      }
    }
    
    if (!requestHandler.auth)
      requestHandler.auth.reset(new CookieAuthentication(persister));
    
    //--- User Statistics ---//

    {
      std::vector<std::string> ports;
      boost::split(ports, varMap["listening_port"].as<std::string>(),
        boost::is_any_of(","));

      std::string filename = varMap.count("stat") ?
        varMap["stat"].as<std::string>() : "";

      requestHandler.userStat.reset(new UserStat(ports, filename));
      requestHandler.userStatSender.reset(new UserStatSender(
        requestHandler.userStat));
    }
    
    //--- Logger settings ---//
    
    auto loglevel =
      cc::util::getLogLevelFromString(varMap["loglevel"].as<std::string>());
    cc::util::StreamLog::setLogLevel(loglevel);

    //--- Plugin settings ---//
    
    if (varMap.count(PLUGIN_DIR))
    {
      fs::path pluginDir = varMap[PLUGIN_DIR].as<std::string>();
      requestHandler.pluginHandler.addDirectory(pluginDir);

      po::options_description pluginOptions = requestHandler.pluginHandler.getOptions();
      options.add(pluginOptions);

      // This time all options have to be known
      varMap.clear();
      parseConfiguration(options, argc, argv, varMap, false);
    }

    if (varMap.count("help"))
    {
      std::cout << options << std::endl;
      return 0;
    }

    requestHandler.pluginHandler.configure(varMap);

    //--- Request Handler params ---//)
    if (varMap.count(DOCUMENT_ROOT))
    {
      requestHandler.documentRoot = varMap[DOCUMENT_ROOT].as<std::string>();
    }

    if (varMap.count(GLOBAL_AUTH_FILE))
    {
      requestHandler.digestPasswdFile = varMap[GLOBAL_AUTH_FILE].as<std::string>();
    }

    //--- Start mongoose server ---//
    
    ThreadedMongoose server;
    
    for (auto optionDescription : mongooseOptions.options())
    {
      auto longName = optionDescription->long_name();
      if (varMap.count(longName)) {
        auto value = varMap[longName].as<std::string>();
        
        server.setOption(longName, value);
      }
    }
    
    std::cout << "Mongoose web server v. " << MONGOOSE_VERSION
      << ", starting on port(s) " << server.getOption("listening_port")
      << " with web root [" << server.getOption("document_root") << "]"
      << std::endl;
    
    server.run(requestHandler);
    
    std::cout << "Exiting, waiting for all threads to finish..." << std::endl;
    
  } catch (const std::exception& ex)
  {
    std::cerr << "Exited with exception: " << ex.what() << std::endl;
    return 1;
  }

  return 0;
}
