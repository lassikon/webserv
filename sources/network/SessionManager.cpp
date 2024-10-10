#include <Client.hpp>
#include <Server.hpp>
#include <SessionManager.hpp>

SessionManager::SessionManager(Server& server) : server(server) {
  LOG_TRACE(Utility::getConstructor(*this));
  clients = server.getClients();
}

SessionManager::~SessionManager(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
}

std::string SessionManager::setExpireTime(void) {
  int lifetime = 3600;  // in seconds
  auto now = std::chrono::system_clock::now();
  auto tt = now + std::chrono::seconds(lifetime);
  std::time_t t = std::chrono::system_clock::to_time_t(tt);
  std::tm* gmt = std::gmtime(&t);
  std::ostringstream oss;
  oss << std::put_time(gmt, "%a, %d-%b-%Y %H:%M:%S GMT");
  return "Expires=" + oss.str();
}

std::string SessionManager::setSessionCookie(void) {
  std::string token, expires;
  token.reserve(tokenLength);
  for (int i = 0; i < tokenLength; i++) {
    token += charSet[rand() % (sizeof(charSet) - 1)];
  }
  std::string cookie = "sessionId=" + token + "; " + setExpireTime() + "; Secure";
  sessionIds.insert(std::make_pair(token, cookie));
  LOG_DEBUG("Generated session cookie:", cookie);
  return cookie;
}

std::string SessionManager::getSessionCookie(std::string sessionToken) {
  LOG_DEBUG("Seeking session token:", sessionToken);
  auto it = sessionIds.find(sessionToken);
  if (it != sessionIds.end()) {
    LOG_DEBUG("Session cookie found:", it->second);
    return it->second;
  }
  LOG_DEBUG("Session has no cookie!");
  return nullptr;
}

bool SessionManager::isSessionCookie(std::string sessionToken) {
  std::string cookie;
  sessionToken = sessionToken.substr(10, 10 + tokenLength);
  auto it = sessionIds.find(sessionToken);
  if (it != sessionIds.end()) {
    cookie = it->second;
    LOG_DEBUG("Session cookie found:", cookie);
    return true;
  }
  return false;
}
