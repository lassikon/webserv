#include <Client.hpp>
#include <NetworkException.hpp>
#include <ServeFileAction.hpp>

void ServeFileAction::execute(Client& client) {
  LOG_TRACE("Serving file");
  std::string path = client.getRes().getReqURI();
  client.getRes().setResStatusCode(200);
  client.getRes().setResStatusMessage("OK");
  checkSetCookie(client, path);
  std::vector<char> ibody = Utility::readFile(path);
  client.getRes().setResBody(ibody);
  std::string fileExt = path.substr(path.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(fileExt);
  client.getRes().addHeader("Content-Type", mimeType);
  client.getRes().addHeader("Content-Length", std::to_string(ibody.size()));
  client.getRes().addHeader("Connection", "keep-alive");
  client.getRes().addHeader("Connection", client.getReq().getHeaders()["Connection"]);
}

void ServeFileAction::checkSetCookie(Client& client, std::string& path) {
  if (isIndexOrDefaultFile(client) == true && !isCookieFound(client, path)) {
    LOG_TRACE("Setting cookie");
    client.getRes().addHeader("Set-Cookie", client.getClientSession().setSessionCookie());
  }
}

bool ServeFileAction::isCookieFound(Client& client, std::string& path) {
  if (client.getReq().getHeaders().find("Cookie") != client.getReq().getHeaders().end()) {
    LOG_DEBUG("Cookie found:", client.getReq().getHeaders()["Cookie"]);
    if (client.getClientSession().isSessionCookie(client.getReq().getHeaders()["Cookie"]) == true) {
      path = std::filesystem::path(path).parent_path().string() + "/welcomeBack.html";
      if (!Utility::isRegularFile(path)) {
        throw httpNotFound(client, "HTTP Error 404 - Not Found");
      }
      if (!Utility::hasReadPerm(path)) {
        throw httpForbidden(client, "HTTP Error 403 - Forbidden");
      }
    } else {
      LOG_INFO("Session cookie not found");
      client.getRes().addHeader("Set-Cookie", client.getClientSession().setSessionCookie());
    }
    return true;
  } else {
    return false;
  }
}

bool ServeFileAction::isIndexOrDefaultFile(Client& client) {
  std::string path = client.getRes().getReqURI();
  for (auto& defFile : client.getRes().getRouteConfig().defaultFile) {
    if (std::filesystem::path(path).filename().string() == defFile) {
      return true;
    }
  }
  return false;
}
