#include <ServeDefaultFileAction.hpp>

void ServeDefaultFileAction::execute(Response& res) {
  LOG_TRACE("Serving default file");
  std::string path = res.getReqURI();
  res.setResStatusCode(200);
  res.setResStatusMessage("OK");
  auto it =
      std::find_if(res.getRouteConfig().defaultFile.begin(),
                   res.getRouteConfig().defaultFile.end(), [path](std::string& defFile) {
                     return std::filesystem::exists(path + defFile);
                   });
  path += *it;
  std::vector<char> ibody = Utility::readFile(path);
  res.setResBody(ibody);
  std::string ext = path.substr(path.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  res.addHeader("Content-Type", mimeType);
  res.addHeader("Content-Length", std::to_string(ibody.size()));
  res.addHeader("Connection", "close");
}