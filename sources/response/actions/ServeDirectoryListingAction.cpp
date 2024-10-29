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
  std::stringstream html;
  std::filesystem::path p(path);
  std::string parentPath = p.parent_path().filename().string();
  html << "<html><body>\n";
  html << "<h1>Directory listing for /" << parentPath << "/</h1>\n";
  html << "<ul>\n";

  std::filesystem::path basePath = path;
  for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
    std::string fileName = entry.path().filename().string();
    std::string relativePath = entry.path().lexically_relative(basePath).string();
    size_t depth = std::count(relativePath.begin(), relativePath.end(),
                              std::filesystem::path::preferred_separator);
    std::string className = "indent";
    std::string style = "style=\"padding-left: " + std::to_string(depth * 20) + "px;\"";

    if (std::filesystem::is_directory(entry)) {
      html << "<li><strong><a href=\"" << relativePath << "/\" " << style << ">"
           << "[" << fileName << "]</a></strong></li>\n";
    } else {
      html << "<li><a href=\"" << relativePath << "\" " << style << ">" << fileName
           << "</a></li>\n";
    }
  }
  html << "</ul>\n";
  html << "</body></html>\n";
  return html.str();
}
