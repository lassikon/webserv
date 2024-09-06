#include <Serve403Action.hpp>

void Serve403Action::execute(Response& res) {
  LOG_TRACE("Serving error 403");
  res.setResStatusCode(403);
  res.setResStatusMessage("Forbidden");
  auto key = res.getServerConfig().pagesDefault.find(403);
  std::string path = key->second;
  std::filesystem::path exePath;
  exePath = Utility::getExePath(exePath);
  if (path.front() == '/') {
    path = path.substr(1, path.size());
  }
  std::filesystem::path errorPath = exePath / path;
  std::string errorPathStr = errorPath.string();
  std::vector<char> ibody = Utility::readFile(errorPathStr);
  res.setResBody(ibody);
  std::string ext = errorPathStr.substr(errorPathStr.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  res.addHeader("Content-Type", mimeType);
  res.addHeader("Content-Length", std::to_string(ibody.size()));
  res.addHeader("Connection", "close");
}