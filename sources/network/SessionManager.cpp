#include <SessionManager.hpp>

SessionManager::SessionManager(void) {
  LOG_TRACE(Utility::getConstructor(*this));
  generateOutfile(sessionsFile, fileName);
  // debugFillSessionsFile();
  readSessionsFromFile();
  // debugPrintSessionsMap();
}

SessionManager::~SessionManager(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  if (sessionsFile.is_open()) {
    sessionsFile.close();
  }
}

void SessionManager::generateOutfile(std::fstream& fs, const char* file) {
  fs.open(file, std::ios::in | std::ios::out | std::ios_base::app);
  if (fs.fail()) {
    LOG_WARN("Could not open file:", file, strerror(errno));
  }
}

void SessionManager::debugFillSessionsFile(void) {
  for (int i = 0; i < tokenLength; i++) {
    sessionsFile << generateSessionId() << "\n";
  }
}

void SessionManager::debugPrintSessionsMap(void) {
  for (const auto& [sessionId, query] : sessionIds) {
    LOG_DEBUG(sessionId, query);
  }
}

void SessionManager::readSessionsFromFile(void) {
  if (!sessionsFile.is_open()) {
    return;
  }
  std::string line, token, query;
  while (std::getline(sessionsFile, line)) {
    token = line.substr(0, line.find_first_of('?'));
    query = line.substr(line.find_first_of('?'));
    sessionIds.insert(std::make_pair(token, query));
  }
}

std::string SessionManager::generateSessionId(void) {
  std::string token;
  token.reserve(tokenLength);
  for (int i = 0; i < tokenLength; ++i) {
    token += charSet[rand() % (sizeof(charSet) - 1)];
  }
  // void = Client &client / QueryString = client.getRes().getReqURI()
  sessionIds.insert(std::make_pair(token, "?name=jon&id=1234"));
  debugPrintSessionsMap();
  sessionsFile << "sessionId=" + token;
  return "sessionId=" + token;
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

// localhost:PORT -> serve login.html
// login as "jon" with id "1234": serve POST welcome.html: URI -> ?name=jon&id=1234
// NOTE: Cookie should be only generated if user logs in!
// in Response -> Cookie: sessionId=GOMKVdDJUSbDcOPDAHEI
// expand URI to Cookie -> sessionId=GOMKVdDJUSbDcOPDAHEI?name=jon&id=1234
// save compined cookie into sessionIds vector
// localhost:3490 -> check Cookie
// Cookie found! serve: GET localhost:3490/?name=jon&id=1234
