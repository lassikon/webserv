#include <Client.hpp>
#include <DeleteHandler.hpp>

std::string DeleteHandler::sanitizePath(std::string path) {
  if (path.empty() || path == "/") {
    return "index.html";
  }
  if (path[0] == '/') {  // Remove slash so appending to rootPath works
    path = path.substr(1);
  }
  return path;
}

void DeleteHandler::executeRequest(Client& client) {
  LOG_INFO("Processing DELETE request for path:", client.getReq().getReqURI());
  LOG_TRACE("DeleteHandler: executingRequest");

  std::string filePath = sanitizePath(client.getReq().getReqURI());
  std::filesystem::path rootPath = client.getRes().getRouteConfig().root;
  std::filesystem::path fullPath = rootPath / filePath;
  LOG_TRACE("fullPath:", fullPath.string());

  bool success = false;
  if (std::filesystem::is_directory(fullPath)) {
    success = std::filesystem::remove_all(fullPath);  // Handles directories
  } else {
    success = std::filesystem::remove(fullPath);  // Handles files
  }

  if (success) {
    LOG_INFO("File/directory deleted successfully");
    client.getRes().setResStatusCode(204);  // No content for successful deletion
    client.getRes().setResStatusMessage("No Content");
    client.getRes().setResBody({});  // Empty body
  } else {
    if (errno == ENOENT) {
      LOG_ERROR("File not found");
      client.getRes().setResStatusCode(404);
      client.getRes().setResStatusMessage("Not Found");
    } else if (errno == EACCES) {
      LOG_ERROR("Permission denied");
      client.getRes().setResStatusCode(403);
      client.getRes().setResStatusMessage("Forbidden");
    } else {
      LOG_ERROR("Error deleting file");
      client.getRes().setResStatusCode(500);
      client.getRes().setResStatusMessage("Internal Server Error");
    }

    client.getRes().addHeader("Content-Type", "text/html");
    std::string htmlResponse = "<html><body><h1>Error processing DELETE request</h1></body></html>";
    std::vector<char> responseBody(htmlResponse.begin(), htmlResponse.end());
    client.getRes().setResBody(responseBody);
  }

  LOG_INFO("DELETE request processed");
}