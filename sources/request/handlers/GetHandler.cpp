#include <GetHandler.hpp>
#include <Client.hpp>

void GetHandler::executeRequest(Client& client) {
  LOG_INFO("Processing GET request for path:", client.getReq().getReqURI());
  LOG_TRACE("GetHandler: executingRequest");
  std::shared_ptr<ProcessTreeBuilder> ptb = std::make_shared<ProcessTreeBuilder>(
    client.getReq(), client.getRes(), client.getRes().getServerConfig());
  //client.getRes().setReqURI(client.getReq().getReqURI());
  root = ptb->buildGetProcessTree();
  root->process(client.getRes());
}