#include <Exception.hpp>

Exception::Exception(void) {}

Exception::~Exception(void) {}

std::string Exception::errCodeToString(ErrorCode errcode) noexcept {
  switch (errcode) {
  case ErrorCode::ArgCount:
    return "Usage: ./webserv OR ./webserv ./<path>/<config>";
  case ErrorCode::ConfigFile:
    return "Could not open config";
  case ErrorCode::NoServer:
    return "Could not run any server";
  default:
    return "Unexpected runtime error";
  }
}
