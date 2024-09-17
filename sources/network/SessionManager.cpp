#include <SessionManager.hpp>

SessionManager::SessionManager(void) {
  Utility::getConstructor(*this);
}
SessionManager::~SessionManager(void) {
  Utility::getDeconstructor(*this);
}
