#ifndef AUTHENTICATION_H
#define	AUTHENTICATION_H

#include <string>
#include <memory>
#include <unordered_map>

#include <boost/bimap/unordered_set_of.hpp>
#include <boost/bimap.hpp>

namespace cc
{
namespace mongoose
{

class Persister
{
public:
  using TokenToUname =
    boost::bimap<boost::bimaps::unordered_set_of<std::string>,
                 boost::bimaps::unordered_set_of<std::string>>;
  
  using TokenToDate = std::unordered_map<std::string, std::string>;
  
  virtual void persist(
    const TokenToUname& tokenToUname,
    TokenToDate& tokenToDate) = 0;
  
  virtual void loadPersisted(
    TokenToUname& tokenToUname,
    TokenToDate& tokenToDate) const = 0;
};
  
class TextFileDatabase : public Persister
{
public:
  TextFileDatabase(const std::string& filename = "");
  
  void persist(
    const TokenToUname& tokenToUname,
    TokenToDate& tokenToDate) override;
  
  void loadPersisted(
    TokenToUname& tokenToUname,
    TokenToDate& tokenToDate) const override;
  
private:
  std::string filename;
};

class Authentication
{
public:
  using TokenToUname = Persister::TokenToUname;
  
  using TokenToDate = std::unordered_map<std::string, std::string>;
  
  Authentication(std::shared_ptr<Persister> persister);
  Authentication(const Authentication&) = delete;
  virtual ~Authentication();
  
  Authentication& operator=(const Authentication&) = delete;
  
  /**
   * This function returns true if there exists an authentication with the given
   * token.
   */
  bool isAuthenticated(const std::string& authToken) const;
  
  /**
   * This function authenticates a user. If authentication is successful then it
   * returns a generated token, or empty string otherwise. Authentication is
   * unsuccessful if the overridden check function returns false.
   */
  std::string authenticate(
    const std::string& username,
    const std::string& password);
  
  virtual bool check(
    const std::string& username,
    const std::string& password) const = 0;
  
private:
  TokenToUname tokenToUname;
  TokenToDate  tokenToDate;
  std::shared_ptr<Persister> persister;
};

class LdapAuthentication : public Authentication
{
public:
  LdapAuthentication(
    const std::string& ldapHost_,
    const std::string& userDn_,
    std::shared_ptr<Persister> persister_);
  
  bool check(
    const std::string& username_,
    const std::string& password_) const override;
  
  void setCertPath(const std::string& path_);
  
private:
  std::string _ldapHost;
  std::string _userDn;
};

class CookieAuthentication : public Authentication
{
public:
  CookieAuthentication(std::shared_ptr<Persister> persister);
  
  bool check(
    const std::string& username,
    const std::string& password) const override;
};

}
}

#endif
