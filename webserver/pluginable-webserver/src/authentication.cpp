#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <ldap.h>

#include "authentication.h"

#include <util/streamlog.h>

namespace cc
{
namespace mongoose
{

TextFileDatabase::TextFileDatabase(const std::string& filename) :
  filename(filename)
{
  
}

void TextFileDatabase::persist(
  const TokenToUname& tokenToUname,
  TokenToDate& tokenToDate)
{
  std::FILE* file = std::fopen(filename.c_str(), "w");
  if (!file)
  {
    SLog(util::ERROR) << "Failed to open file: " << filename;
    return;
  }
  
  char buffer[20];
  std::time_t rawtime;
  std::time(&rawtime);
  strftime(buffer, 20, "%F %T", std::localtime(&rawtime));
  
  for (const auto& tok : tokenToUname.left)
  {
    if (tokenToDate.find(tok.first) == tokenToDate.end())
    {
      tokenToDate[tok.first] = buffer;
    }
    
    std::fprintf(file, "%s %s %s\n",
      tok.first.c_str(), tok.second.c_str(), tokenToDate[tok.first].c_str());
  }

  std::fclose(file);
}

void TextFileDatabase::loadPersisted(
  TokenToUname& tokenToUname,
  TokenToDate& tokenToDate) const
{
  tokenToUname.clear();
  
  std::ifstream file(filename.c_str());
  
  std::string line;
  while (std::getline(file, line))
  {
    std::size_t pos1 = line.find(' ');
    std::string token = line.substr(0, pos1);
    
    std::size_t pos2 = line.find(' ', pos1 + 1);
    std::string uname = line.substr(token.length() + 1, pos2 - pos1 - 1);
    
    std::string date = line.substr(pos2 + 1);
    
    tokenToUname.insert(TokenToUname::value_type(token, uname));
    tokenToDate[token] = date;
  }
  
  file.close();
}

Authentication::Authentication(std::shared_ptr<Persister> persister) :
  persister(persister)
{
  persister->loadPersisted(tokenToUname, tokenToDate);
}

Authentication::~Authentication()
{
  persister->persist(tokenToUname, tokenToDate);
}

bool Authentication::isAuthenticated(const std::string& authToken) const
{
  return tokenToUname.left.find(authToken) != tokenToUname.left.end();
}

std::string Authentication::authenticate(
  const std::string& username,
  const std::string& password)
{
  if (!check(username, password))
    return "";
  
  auto it = tokenToUname.right.find(username);
  if (it != tokenToUname.right.end())
    return it->second;

  std::srand(std::time(0));

  const std::string characters = "abcdefghijklmnopqrstuvwxyz" \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

  std::string token;

  for (int i = 0; i < 10; ++i)
    token += characters[std::rand() % characters.size()];

  tokenToUname.insert(TokenToUname::value_type(token, username));
  persister->persist(tokenToUname, tokenToDate);

  return token;
}

LdapAuthentication::LdapAuthentication(
  const std::string& ldapHost_,
  const std::string& userDn_,
  std::shared_ptr<Persister> persister_)
: Authentication(persister_)
, _ldapHost(ldapHost_),
  _userDn(userDn_)
{
  // Uncomment this if you want some ldap log
  /*
    int ldapDebugLevel = -1;
    ldap_set_option(0, LDAP_OPT_DEBUG_LEVEL, &ldapDebugLevel);
  */

  SLog() << "LDAP host: " << _ldapHost;
  SLog() << "LDAP dn: " << _userDn;

  if (_ldapHost.empty())
  {
    throw std::logic_error("LDAP host URL must be given");
  }
    
  if (_userDn.find("{}") == std::string::npos)
  {
    throw std::logic_error("LDAP user DN must contain {} where username is " \
      "substituted");
  }
  
  setCertPath("");
}

bool LdapAuthentication::check(
  const std::string& username,
  const std::string& password) const
{
  if (_ldapHost.empty())
    throw std::logic_error("LDAP host URL must be set");
  if (_userDn.empty())
    throw std::logic_error("LDAP user DN must be set");
  
  struct berval cred;
  if (password.empty())
  {
    SLog() << "LDAP: empty password!";
    return false;
  }
  else
  {
    cred.bv_len = password.size();
    cred.bv_val = const_cast<char*>(password.c_str());
  }

  std::string dn = _userDn;
  dn.replace(dn.find("{}"), 2, username);
  
  LDAP* ldap = nullptr;
  int   ldapError;

  ldapError = ldap_initialize(&ldap, _ldapHost.c_str());
  if (ldapError != LDAP_SUCCESS)
  {
    std::string errMsg = "LDAP connection fail: " + _ldapHost;

    const char* errCStr = ldap_err2string(ldapError);
    if (errCStr)
    {
      errMsg = "LDAP connection failed to: " + _ldapHost +
        "! With error: " + std::string(errCStr);
    }
    else
    {
      errMsg = "LDAP connection fail: " + _ldapHost;
    }
    
    SLog() << errMsg;
    throw std::runtime_error(errMsg);
  }
  
  bool checkResult = true;
  ldapError = ldap_sasl_bind_s(ldap, dn.c_str(), LDAP_SASL_SIMPLE, &cred,
    nullptr, nullptr, nullptr);
  if (ldapError != LDAP_SUCCESS)
  {
    std::string errMsg = "Error in sasl bind! ";

    const char* errCStr = ldap_err2string(ldapError);
    if (errCStr)
    {
      errMsg += errCStr;
    }

    SLog() << errMsg;
    checkResult = false;
  }
  
  SLog() << "LdapAuthentication::check result " << checkResult;
  
  ldap_destroy(ldap);

  return checkResult;
}

void LdapAuthentication::setCertPath(const std::string& path_)
{
  SLog() << "LDAP setCertPath " << path_;

  int ldapError;
  if (path_.empty())
  {
    SLog() << "LDAP setting LDAP_OPT_X_TLS_NEVER";

    int opt = LDAP_OPT_X_TLS_NEVER;
    ldapError = ldap_set_option(0, LDAP_OPT_X_TLS_REQUIRE_CERT, &opt);
  }
  else
  {
    SLog() << "LDAP setting LDAP_OPT_X_TLS_DEMAND";

    int opt = LDAP_OPT_X_TLS_DEMAND;
    ldapError = ldap_set_option(0, LDAP_OPT_X_TLS_REQUIRE_CERT, &opt);
    if (ldapError == LDAP_SUCCESS)
    {
      SLog() << "LDAP setting certificate file to " << path_;
      ldapError = ldap_set_option(0, LDAP_OPT_X_TLS_CACERTFILE, path_.c_str());
    }
  }
  
  if (ldapError != LDAP_SUCCESS)
  {
    const char* descStr = ldap_err2string(ldapError);
    std::string desc;
    if (descStr)
    {
      desc = descStr;
    }
    
    std::string errMsg = "LDAP set option failed: "  + desc;

    SLog() << errMsg;
    throw std::runtime_error(desc);
  }

  SLog() << "LDAP setCertPath successful";
}

CookieAuthentication::CookieAuthentication(std::shared_ptr<Persister> persister)
  : Authentication(persister)
{
  
}

bool CookieAuthentication::check(const std::string&, const std::string&) const
{
  return true;
}

}
}
