#include <SessionManager.hpp>

SessionManager::SessionManager(void) : lifetime(COOKIE_TIMEOUT) {
  LOG_TRACE(Utility::getConstructor(*this));
}

SessionManager::~SessionManager(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
}

void SessionManager::debugPrintTokens(void) {
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
  std::random_device randomSeed;
  std::mt19937 generator(randomSeed());

  // Create a distribution to uniformly select from all characters
  std::uniform_int_distribution<> distribute(0, charSet.size() - 1);
  for (size_t i = 0; i < tokenLength; i++) {
    token += charSet[distribute(generator)];
  }
  std::string cookie = "sessionId=" + token + "; " + setExpireTime() + "; Secure";
  LOG_DEBUG("Generated session cookie:", cookie);
  auto now = std::chrono::system_clock::now();
  sessionIds.insert(std::make_pair(token, now + lifetime));
  return cookie;
}

bool SessionManager::isSessionCookie(std::string sessionToken) {
  sessionToken = sessionToken.substr(10, 10 + tokenLength);
  LOG_DEBUG("Checking session token:", sessionToken);
  auto it = sessionIds.find(sessionToken);
  if (it != sessionIds.end()) {
    LOG_DEBUG("Session cookie found!");
    return true;
  }
  return false;
}

void SessionManager::checkExpiredCookies(void) {
  for (auto it = sessionIds.begin(); it != sessionIds.end();) {
    if (std::chrono::system_clock::now() > it->second) {
      LOG_INFO("Session cookie expired:", it->first);
      it = sessionIds.erase(it);
    } else {
      it++;
    }
  }
}