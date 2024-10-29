#include <Request.hpp>

Request::Request() {
  LOG_TRACE(Utility::getConstructor(*this));
}

Request::~Request() {
  LOG_TRACE(Utility::getDeconstructor(*this));
}