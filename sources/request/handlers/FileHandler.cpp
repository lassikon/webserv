#include <FileHandler.hpp>

FileHandler::FileHandler(void) {
  Utility::getConstructor(*this);
}

FileHandler::~FileHandler(void) {
  Utility::getDeconstructor(*this);
}
