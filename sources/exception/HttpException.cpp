#include <HttpException.hpp>

void HttpException::setResponseAttributes(void) {
  res.setResStatusCode(errorCode);
  res.setResStatusMessage(message);
  auto key = res.getServerConfig().pagesCustom.find(errorCode);
  if (key == res.getServerConfig().pagesCustom.end()) {
    key = res.getServerConfig().pagesDefault.find(errorCode);
  }
  std::string path = key->second;
  std::filesystem::path exePath;
  exePath = Utility::getExePath(exePath);
  if (path.front() == '/') {
    path = path.substr(1, path.size());
  }
  std::filesystem::path errorPath = exePath / path;
  std::string errorPathStr = errorPath.string();
  
  if (!std::filesystem::exists(errorPath) || !isValid(errorPathStr)) {
    std::string body("<html><body><h1>404 Not Found, error page not found!</h1></body></html>");
    std::vector<char> ibody(body.begin(), body.end());
    res.setResStatusCode(404);
    res.setResStatusMessage("Not Found");
    res.setResBody(ibody);
    res.addHeader("Content-Type", "text/html");
    res.addHeader("Connection", "keep-alive");
    res.addHeader("Content-Length", std::to_string(ibody.size()));
    return;
  }
  std::vector<char> ibody = Utility::readFile(errorPathStr);
  res.setResBody(ibody);
  std::string ext = errorPathStr.substr(errorPathStr.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  //res.addHeader("Cache-Control", "max-age=3600, must-revalidate");
  res.addHeader("Content-Type", mimeType);
  res.addHeader("Content-Length", std::to_string(ibody.size()));
  res.addHeader("Connection", "keep-alive");
}

bool HttpException::isValid(std::string path) const {
  struct stat s;
  if (!stat(path.c_str(), &s) && S_ISREG(s.st_mode) && !access(path.c_str(), X_OK)) {
    return true;
  }
  return false;
}