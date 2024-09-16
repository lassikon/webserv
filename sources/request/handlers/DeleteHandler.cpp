#include <Client.hpp>
#include <DeleteHandler.hpp>

void DeleteHandler::executeRequest(Client& client) {
  LOG_INFO("Processing DELETE request for path:", client.getReq().getReqURI());
  LOG_TRACE("DeleteHandler: executingRequest");
}

//std::filesystem::remove(filePath)