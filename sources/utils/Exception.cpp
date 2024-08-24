#include <Exception.hpp>

Exception::Exception(void) {}

Exception::~Exception(void) {}

std::string Exception::errCodeToString(const ErrorCode &errCode) noexcept {
  switch (errCode) {
  case ErrorCode::ArgCount:
    return ERR_MSG_USAGE;
  case ErrorCode::ConfigFile:
    return ERR_MSG_CONFIG;
  case ErrorCode::NoServer:
    return ERR_MSG_NOSERV;
  default:
    return "Unexpected runtime error";
  }
}
