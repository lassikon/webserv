
#include <Client.hpp>
#include <ServeQueryAction.hpp>

void ServeQueryAction::execute(Client& client) {
  LOG_TRACE("Serving Query");
  std::string decode = UrlEncoder::decode(client.getReq().getQuery());
  auto it = decode.find("=");
  if (it != std::string::npos) {
    std::string value = decode.substr(it + 1, decode.size() - it);
    std::string lower;
    for (auto& c : value) {
      lower += std::tolower(c);
    }
    if (lower.compare("network") != 0) {
      serveWrongQuery(client);
      return;
    }
  } else {
    std::string path = client.getRes().getReqURI();
    client.getRes().setResStatusCode(200);
    client.getRes().setResStatusMessage("OK");
    std::vector<char> ibody = Utility::readFile(path);
    client.getRes().setResBody(ibody);
    std::string ext = path.substr(path.find_last_of(".") + 1);
    std::string mimeType = Utility::getMimeType(ext);
    client.getRes().addHeader("Content-Type", mimeType);
    client.getRes().addHeader("Content-Length", std::to_string(ibody.size()));
    client.getRes().addHeader("Connection", client.getReq().getHeaders()["Connection"]);
  }
}

void ServeQueryAction::serveWrongQuery(Client& client) {
  LOG_TRACE("Serving wrong query");
  client.getRes().setResStatusCode(301);
  client.getRes().setResStatusMessage("Moved Permanently");
  client.getRes().addHeader("Location", "https://www.youtube.com/watch?v=r94vuvwUSkY");
  client.getRes().addHeader("Connection", client.getReq().getHeaders()["Connection"]);
}
