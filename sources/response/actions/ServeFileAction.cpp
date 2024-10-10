#include <Client.hpp>
#include <NetworkException.hpp>
#include <ServeFileAction.hpp>

void ServeFileAction::execute(Client& client) {
  LOG_TRACE("Serving file");
  std::string path = client.getRes().getReqURI();
  client.getRes().setResStatusCode(200);
  client.getRes().setResStatusMessage("OK");
  ifCookie(client, path);
  std::vector<char> ibody = Utility::readFile(path);
  client.getRes().setResBody(ibody);
  std::string ext = path.substr(path.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  // client.getRes().addHeader("Cache-Control", "max-age=3600, must-revalidate");
  client.getRes().addHeader("Content-Type", mimeType);
  client.getRes().addHeader("Content-Length", std::to_string(ibody.size()));
  client.getRes().addHeader("Connection", "keep-alive");
  client.getRes().addHeader("Connection", client.getReq().getHeaders()["Connection"]);
}

void ServeFileAction::ifCookie(Client& client, std::string& path) {
  if (isIndexOrDefaultFile(client) == true) {
    if (!isCookieFound(client, path))
      LOG_TRACE("Setting cookie");
    client.getRes().addHeader("Set-Cookie", client.getClientSession().setSessionCookie());
  }
}

bool ServeFileAction::isCookieFound(Client& client, std::string& path) {
  if (client.getReq().getHeaders().find("Cookie") != client.getReq().getHeaders().end()) {
    LOG_DEBUG("Cookie found:", client.getReq().getHeaders()["Cookie"]);
    if (client.getClientSession().isSessionCookie(client.getReq().getHeaders()["Cookie"]) == true) {
      std::filesystem::path p(path);
      path = p.parent_path().string() + "/welcomeBack.html";
      if (!isExist(path)) {
        throw httpNotFound(client, "HTTP Error 404 - Not Found");
      }
      if (!isPerm(path)) {
        throw httpForbidden(client, "HTTP Error 403 - Forbidden");
      }
    } else {
      LOG_WARN("Session cookie not found");
      client.getRes().addHeader("Set-Cookie", client.getClientSession().setSessionCookie());
    }
    return true;
  }
  return false;
}

bool ServeFileAction::isIndexOrDefaultFile(Client& client) {
  std::string path = client.getRes().getReqURI();
  std::filesystem::path p(path);
  if (p.filename().string() == "index.html" || p.filename().string() == "index.htm") {
    return true;
  }
  for (auto& defFile : client.getRes().getRouteConfig().defaultFile) {
    if (p.filename().string() == defFile) {
      return true;
    }
  }
  return false;
}

bool ServeFileAction::isExist(std::string path) const {
  struct stat s;
  if (!stat(path.c_str(), &s) && S_ISREG(s.st_mode)) {
    return true;
  }
  return false;
}

bool ServeFileAction::isPerm(std::string path) const {
  if (!access(path.c_str(), R_OK)) {
    return true;
  }
  return false;
  ;
}
