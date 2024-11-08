#include <Response.hpp>

Response::Response() {
  LOG_TRACE(Utility::getConstructor(*this));
}

Response::~Response() {
  LOG_TRACE(Utility::getDeconstructor(*this));
}

void Response::makeResponse(void) {
  std::ostringstream oss;
  LOG_TRACE("Making response");
  oss << "HTTP/1.1 " << resStatusCode << " " << resStatusMessage << "\r\n";
  for (auto& [key, value] : resHeaders) {
    oss << key << ": " << value << "\r\n";
  }
  oss << "\r\n";
  std::string buffer = oss.str();
  resContent = std::vector<char>(buffer.begin(), buffer.end());
  resContent.insert(resContent.end(), resBody.begin(), resBody.end());
}

void Response::makeBodytoCgi(void) {
  resContent.insert(resContent.end(), resBody.begin(), resBody.end());
}
