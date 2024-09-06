#include <ServeDirectoryListingAction.hpp>

//not coded
void ServeDirectoryListingAction::execute(Response& res) {
  LOG_TRACE("Serving directory listing");
  (void)res;
 /*  std::string path = res.getReqURI();
  res.setResStatusCode(200);
  res.setResStatusMessage("OK");
  std::vector<char> ibody = Utility::readFile(path);
  res.setResBody(ibody);
  std::string ext = path.substr(path.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  res.addHeader("Content-Type", mimeType);
  res.addHeader("Content-Length", std::to_string(ibody.size()));
  res.addHeader("Connection", "close"); */
}