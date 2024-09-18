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
  return token;
}

void SessionManager::generateOutfile(std::fstream& fs, const char* file) {
  fs.open(file, std::ios_base::app);
  if (fs.fail()) {
    LOG_WARN("Could not open file:", file, strerror(errno));
  }
}
