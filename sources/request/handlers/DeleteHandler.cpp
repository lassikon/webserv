#include <Client.hpp>
#include <DeleteHandler.hpp>
#include <NetworkException.hpp>

std::string DeleteHandler::sanitizePath(std::string path) {
  if (path.empty() || path == "/") {
    return "index.html";
  }
  if (path[0] == '/') {  // Remove slash so appending to rootPath works
    path = path.substr(1);
  }
  return path;
}

void DeleteHandler::deleteFile(std::filesystem::path fullPath, Client& client) {
  if (!std::filesystem::exists(fullPath)) {
    throw httpNotFound(client, "The requested file was not found");
  }
  std::uintmax_t numRemoved = std::filesystem::remove_all(fullPath) ;
  if (numRemoved > 0) {
    LOG_INFO("File/directory deleted successfully, number of items removed: ", numRemoved);
    client.getRes().setResStatusCode(204);  // No content for successful deletion
    client.getRes().setResStatusMessage("No Content");
    client.getRes().setResBody({});
    return;
  }
  throw httpForbidden(client, "Unable to delete the file/directory due to permission issues");
}

void DeleteHandler::executeRequest(Client& client) {
  LOG_INFO("Processing DELETE request for path:", client.getReq().getReqURI());
  LOG_TRACE("DeleteHandler: executingRequest");
  std::string filePath = sanitizePath(client.getReq().getReqURI());
  std::filesystem::path rootPath = client.getRes().getRouteConfig().root;
  std::filesystem::path fullPath = rootPath / filePath;
  LOG_TRACE("fullPath:", fullPath.string());
  NetworkException::tryCatch(&DeleteHandler::deleteFile, this, fullPath, std::ref(client));
}