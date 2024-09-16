#include <Response.hpp>

Response::Response() {
  LOG_DEBUG(Utility::getConstructor(*this));
}

Response::~Response() {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}

void Response::makeResponse(void) {
  LOG_TRACE("Making response");
  std::ostringstream oBuf;
  oBuf << "HTTP/1.1 " << resStatusCode << " " << resStatusMessage << "\r\n";
  for (auto& [key, value] : resHeaders) {
    oBuf << key << ": " << value << "\r\n";
  }
  oBuf << "\r\n";
  std::string oBufStr = oBuf.str();
  resContent = std::vector<char>(oBufStr.begin(), oBufStr.end());
  resContent.insert(resContent.end(), resBody.begin(), resBody.end());
}
