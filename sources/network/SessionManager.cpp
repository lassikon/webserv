#include <Client.hpp>
#include <Server.hpp>
#include <SessionManager.hpp>

SessionManager::SessionManager(Server& server) : server(server) {
  LOG_TRACE(Utility::getConstructor(*this));
  generateOutfile(sessionsFile, fileName);
  clients = server.getClients();
}

SessionManager::~SessionManager(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  if (sessionsFile.is_open()) {
    sessionsFile.close();
  }
}

void SessionManager::generateOutfile(std::fstream& fs, const char* file) {
  fs.open(file, std::ios::in | std::ios::out | std::ios_base::app);
  if (fs.fail() && !errorLogged) {
    LOG_WARN("Could not open file:", file, strerror(errno));
    errorLogged = true;
  }
}

void SessionManager::debugPrintSessionsMap(void) {
  for (const auto& [sessionId, query] : sessionIds) {
    LOG_INFO(sessionId, query);
  }
}

void SessionManager::readSessionsFromFile(void) {
  if (!sessionsFile.is_open()) {
    return;
  } else {
    std::string line, token, query;
    while (std::getline(sessionsFile, line)) {
      token = line.substr(0, line.find_first_of('?'));
      query = line.substr(line.find_first_of('?'));
      sessionIds.insert(std::make_pair(token, query));
    }
  }
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
  if (sessionsFile.is_open()) {
    sessionsFile << (cookie + "\n");
  }
  return cookie;
}

std::string SessionManager::getSessionCookie(std::string sessionToken) {
  LOG_DEBUG("Current session token is:", sessionToken);
  for (const auto& [token, cookie] : sessionIds) {
    if (token == sessionToken) {
      return cookie;
    }
  }
  return {};
}
