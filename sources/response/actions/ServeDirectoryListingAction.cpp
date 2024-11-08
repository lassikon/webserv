#include <Client.hpp>
#include <ServeDirectoryListingAction.hpp>

void ServeDirectoryListingAction::execute(Client& client) {
  LOG_TRACE("Serving directory listing");
  std::string path = client.getRes().getReqURI();
  std::string html = makeDirectoryListing(path);
  client.getRes().setResStatusCode(200);
  client.getRes().setResStatusMessage("OK");
  std::vector<char> ibody = std::vector<char>(html.begin(), html.end());
  client.getRes().setResBody(ibody);
  client.getRes().addHeader("Content-Type", "text/html");
  client.getRes().addHeader("Content-Length", std::to_string(ibody.size()));
  client.getRes().addHeader("Connection", client.getReq().getHeaders()["Connection"]);
}
std::string ServeDirectoryListingAction::makeDirectoryListing(std::string path) {
  namespace fs = std::filesystem;
  std::string parentPath = fs::path(path).parent_path().filename().string();
  std::stringstream html;
  html << "<html><body>\n";
  html << "<h1>Directory listing for /" << parentPath << "/</h1>\n";
  html << "<ul>\n";
  for (const auto& entry : fs::recursive_directory_iterator(path)) {
    std::string fileName = entry.path().filename().string();
    std::string relativePath = entry.path().lexically_relative(fs::path(path)).string();
    size_t depth = std::count(relativePath.begin(), relativePath.end(), fs::path::preferred_separator);
    std::string style = "style=\"padding-left: " + std::to_string(depth * 20) + "px;\"";
    if (fs::is_directory(entry)) {
      html << "<li><strong><a href=\"" << relativePath << "/\" " << style << ">" << "[" << fileName << "]</a></strong></li>\n";
    } else {
      html << "<li><a href=\"" << relativePath << "\" " << style << ">" << fileName << "</a></li>\n";
    }
  }
  html << "</ul>\n";
  html << "</body></html>\n";
  return html.str();
}
