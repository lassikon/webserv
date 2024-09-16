#include <Client.hpp>
#include <GetHandler.hpp>

void GetHandler::executeRequest(Client& client) {
  if (client.getRes().getResStatusCode() != 0) {
    return;
  }
  LOG_INFO("Processing GET request for path:", client.getReq().getReqURI());
  LOG_TRACE("GetHandler: executingRequest");
  std::shared_ptr<ProcessTreeBuilder> ptb = std::make_shared<ProcessTreeBuilder>(
    client, client.getRes().getServerConfig());
  //client.getRes().setReqURI(client.getReq().getReqURI());
  root = ptb->buildGetProcessTree();
  root->process(client);
}
