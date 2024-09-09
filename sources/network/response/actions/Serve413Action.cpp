#include <Serve413Action.hpp>

void Serve413Action::execute(Response& res) {
  LOG_TRACE("Serving error 413");
  res.setResStatusCode(413);
  res.setResStatusMessage("Payload Too Large");
   auto key = res.getServerConfig().pagesCustom.find(413);
  if (key == res.getServerConfig().pagesCustom.end()) {
    key = res.getServerConfig().pagesDefault.find(413);
  }
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
  res.addHeader("Cache-Control", "max-age=3600, must-revalidate");
  res.addHeader("Content-Type", mimeType);
  res.addHeader("Content-Length", std::to_string(ibody.size()));
  res.addHeader("Connection", "close");
}
