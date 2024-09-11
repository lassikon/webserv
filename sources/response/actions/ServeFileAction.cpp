#include <ServeFileAction.hpp>

void ServeFileAction::execute(Response& res) {
  LOG_TRACE("Serving file");
  std::string path = res.getReqURI();
  res.setResStatusCode(200);
  res.setResStatusMessage("OK");
  std::vector<char> ibody = Utility::readFile(path);
  res.setResBody(ibody);
  std::string ext = path.substr(path.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  res.addHeader("Cache-Control", "max-age=3600, must-revalidate");
  res.addHeader("Content-Type", mimeType);
  res.addHeader("Content-Length", std::to_string(ibody.size()));
  res.addHeader("Connection", "close");
}