#include <random>
#include <sstream>
#include <vector>

#include <boost/algorithm/string.hpp>

#include <util/hash.h>
#include <util/util.h>

#include "authentication.h"
#include "sessionmanager.h"

#define CODECOMPASS_SESSION_COOKIE "CodeCompass_SESH"

/**
 * The number of sessions to exist in memory before automatic clearing of stale
 * sessions is performed.
 */
static constexpr std::size_t SessionClearThreshold = 16;

namespace cc
{
namespace webserver
{

SessionManager::SessionManager(const Authentication* authEngine_)
  : _authEngine(authEngine_)
{
  if (!isRequiringAuthentication())
    // Create a default session for services to be able to use, which is shared
    // between all users.
    _sessions.emplace(CODECOMPASS_SESSION_COOKIE, Session{CODECOMPASS_SESSION_COOKIE, "Anonymous"});
}

bool SessionManager::isRequiringAuthentication() const
{
  return _authEngine->isEnabled();
}

const std::string& SessionManager::getAuthPrompt() const
{
  return _authEngine->getAuthPrompt();
}

std::string SessionManager::getSessionCookieName() const
{
  return CODECOMPASS_SESSION_COOKIE;
}

Session* SessionManager::getSessionCookie(const char* cookieHeader_)
{
  if (!isRequiringAuthentication())
  {
    const std::lock_guard<std::mutex> lock{_sessionMapLock};
    auto it = _sessions.find(CODECOMPASS_SESSION_COOKIE);
    return &it->second;
  }

  if (!cookieHeader_)
    return nullptr;

  std::string identifier;

  std::vector<std::string> splitResult;
  boost::algorithm::split(
    splitResult, cookieHeader_, [](const char ch) { return ch == ';'; });
  for (const std::string& cookie : splitResult)
  {
    if (cookie.find(CODECOMPASS_SESSION_COOKIE) != std::string::npos)
    {
      std::string::size_type equalLoc =
        cookie.find('=', strlen(CODECOMPASS_SESSION_COOKIE));
      if (equalLoc != std::string::npos)
      {
        identifier = cookie.substr(equalLoc + 1, std::string::npos);
        break;
      }
    }
  }

  if (identifier.empty())
    return nullptr;

  const std::lock_guard<std::mutex> lock{_sessionMapLock};
  auto it = _sessions.find(identifier);
  if (it == _sessions.end())
    return nullptr;
  return &it->second;
}

void SessionManager::destroySessionCookie(Session* session_)
{
  if (!session_ || session_->sessId == CODECOMPASS_SESSION_COOKIE)
    return;

  const std::lock_guard<std::mutex> lock{_sessionMapLock};
  _sessions.erase(session_->sessId);
  cleanupOldSessions();
}

bool SessionManager::isValid(const Session* session_) const
{
  if (!isRequiringAuthentication())
    // In non-authenticating mode, consider everything valid anonymous access.
    return true;
  if (!session_)
    return false;

  SessionTimePoint expiry =
    session_->lastHit() +
    std::chrono::seconds(_authEngine->getSessionLifetime());
  if (std::chrono::steady_clock::now() >= expiry)
    // The session has expired (configured number of seconds passed since it was
    // last seen.)
    return false;

  return true;
}

Session* SessionManager::authenticateUserWithNameAndPassword(
  const std::string& username_, const std::string& password_)
{
  if (!_authEngine->authenticateUsernamePassword(username_, password_))
    return nullptr;

  static std::random_device rnd;
  std::mt19937 random{rnd()};
  std::uniform_int_distribution<> distribution{1}; // Generate [1, MAX].

  std::ostringstream os;
  os << username_ << distribution(random) << CODECOMPASS_SESSION_COOKIE
     << util::getCurrentDate() << distribution(random);
  std::string id = util::sha1Hash(os.str());

  const std::lock_guard<std::mutex> lock{_sessionMapLock};
  auto it = _sessions.emplace(id, Session{id, username_});
  return &it.first->second;
}

/**
 * Cleans up old sessions if the number of stored sessions is above a threshold.
 * This method expects to be called in a locked context.
 */
void SessionManager::cleanupOldSessions()
{
  if (_sessions.size() > SessionClearThreshold)
  {
    for (auto it = _sessions.begin(); it != _sessions.end();)
    {
      if (!isValid(&it->second))
        it = _sessions.erase(it);
      else
        ++it;
    }
  }
}

} // namespace webserver
} // namespace cc
