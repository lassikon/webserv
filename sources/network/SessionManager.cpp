#include <SessionManager.hpp>

SessionManager::SessionManager(void) {
  Utility::getConstructor(*this);
}

SessionManager::~SessionManager(void) {
  Utility::getDeconstructor(*this);
  generateOutfile(sessions, fileName);
}

std::string SessionManager::generateSessionId(void) {
  std::string token;
  token.reserve(tokenLength);
  for (int i = 0; i < tokenLength; ++i) {
    token += charSet[rand() % (sizeof(charSet) - 1)];
  }
  sessionIds.insert(std::make_pair(token, "QueryString goes here?"));
  return "sessionId=" + token;
}

// localhost:3490/login.html
// URI = ?name=jon&id=1234 = querystring
// "Set-Cookie: sessionId=dfgf8dsf7g9fdsy98he"
// localhost:3490/login.html?name=jon&id=1234

void SessionManager::generateOutfile(std::fstream& fs, const char* file) {
  fs.open(file, std::ios_base::app);
  if (fs.fail()) {
    LOG_WARN("Could not open file:", file, strerror(errno));
  }
}

bool SessionManager::sessionIdExists(std::string session) {
  auto it = sessionIds.find(session);
  if (it != sessionIds.end()) {
    return true;
  } else {
    return false;
  }
}

std::string SessionManager::getSessionId(std::string newSession) {
  for (const auto& [session, query] : sessionIds) {
    if (session == newSession) {
      return query;
    }
  }
  return nullptr;
}
