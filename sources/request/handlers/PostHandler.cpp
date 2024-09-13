#include <PostHandler.hpp>
#include <Client.hpp>

void PostHandler::executeRequest(Client& client) {
  LOG_TRACE("PostHandler: executingRequest");
  LOG_INFO("Processing POST request for path:", client.getReq().getReqURI());
}