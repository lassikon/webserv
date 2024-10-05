#include <Client.hpp>
#include <ServeFileAction.hpp>

void ServeFileAction::execute(Client& client) {
  LOG_TRACE("Serving file");
  std::string path = client.getRes().getReqURI();
  client.getRes().setResStatusCode(200);
  client.getRes().setResStatusMessage("OK");
  std::vector<char> ibody = Utility::readFile(path);
  client.getRes().setResBody(ibody);
  std::string ext = path.substr(path.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  // client.getRes().addHeader("Cache-Control", "max-age=3600, must-revalidate");
  client.getRes().addHeader("Content-Type", mimeType);
  client.getRes().addHeader("Content-Length", std::to_string(ibody.size()));
  client.getRes().addHeader("Connection", "keep-alive");
  client.getRes().addHeader("Connection", client.getReq().getHeaders()["Connection"]);
  // client.getRes().addHeader("Cookie", "384yNSDFu29rrwASD");
}
