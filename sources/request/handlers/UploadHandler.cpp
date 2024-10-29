#include <Client.hpp>

#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>

#include <UploadHandler.hpp>

void UploadHandler::executeRequest(Client& client) {
  if (client.getRes().getResStatusCode() != 0) {
    return;
  }
  LOG_INFO("Processing UPLOAD request for path:", client.getReq().getReqURI());
}
