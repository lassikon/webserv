#include <Client.hpp>
#include <Request.hpp>

Request::Request() {
  LOG_DEBUG(Utility::getConstructor(*this));
}

Request::~Request() {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}