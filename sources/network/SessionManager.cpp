#include <SessionManager.hpp>

SessionManager::SessionManager(void) : lifetime(3600) {
  LOG_TRACE(Utility::getConstructor(*this));
}

SessionManager::~SessionManager(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
}

void SessionManager::debugPrintSessionsMap(void) {
  for (const auto& [token, cookie] : sessionIds) {
    LOG_DEBUG("Cookie token:", token);
  }
}

std::string SessionManager::setExpireTime(void) {
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
  auto now = std::chrono::system_clock::now();
  sessionIds.insert(std::make_pair(token, now + lifetime));
  LOG_DEBUG("Generated session cookie:", cookie);
  return cookie;
}

bool SessionManager::isSessionCookie(std::string sessionToken) {
  std::string cookie;
  sessionToken = sessionToken.substr(10, 10 + tokenLength);
  LOG_DEBUG("Checking session token:", sessionToken);
  auto it = sessionIds.find(sessionToken);
  if (it != sessionIds.end()) {
    LOG_DEBUG("Session cookie found!");
    return true;
  }
  return false;
}
