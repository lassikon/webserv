#include <SessionManager.hpp>
#include <Server.hpp>
#include <Client.hpp>

SessionManager::SessionManager(Server &server) : server(server) {
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
    LOG_DEBUG(sessionId, query);
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

std::string SessionManager::setSessionCookie(Response& response) {
  std::vector<char> vec = response.getResBody();
  std::string token, query(vec.data(), vec.size());
  token.reserve(tokenLength);
  for (int i = 0; i < tokenLength; i++) {
    token += charSet[rand() % (sizeof(charSet) - 1)];
  }
  std::cout << "\n\n\n" << query << "\n\n\n";  // DEBUG:
  sessionIds.insert(std::make_pair(token, query));
  sessionsFile << ("sessionId=" + token + query + "\n");
  return "sessionId=" + token;
}

std::string SessionManager::getSessionQuery(std::string currentSession) {
  for (const auto& [session, query] : sessionIds) {
    if (session == currentSession) {
      return query;
    }
  }
  return {};
}

std::string SessionManager::getSessionCookie(std::string currentSession) {
  for (const auto& [session, query] : sessionIds) {
    if (session == currentSession) {
      return session;
    }
  }
  return {};
}

// localhost:3490 -> serve login.html
// login as "jon" with id "1234": serve POST welcome.html: URI -> ?name=jon&id=1234
// NOTE: Cookie should be only generated if user logs in!
// in Response -> Cookie: sessionId=GOMKVdDJUSbDcOPDAHEI
// expand URI to Cookie -> sessionId=GOMKVdDJUSbDcOPDAHEI?name=jon&id=1234
// save compined cookie into sessionIds vector/map
// reconnect to localhost:3490 -> check Cookies...
// Cookie found! serve welcome page: GET localhost:3490?name=jon&id=1234
