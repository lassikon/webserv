#include <Request.hpp>

Request::Request() {
  LOG_TRACE(Utility::getConstructor(*this));
}

Request::~Request() {
  LOG_TRACE(Utility::getDeconstructor(*this));
}
size_t Request::getContentLength(void) const {
  auto it = reqHeaders.find("Content-Length");
  if (it == reqHeaders.end()) {
    return 0;
  }
  try {
    return std::stoi(it->second);
  } catch (std::exception& e) {
    LOG_ERROR("Failed to convert Content-Length to int:", e.what());
    return 0;
  }
}