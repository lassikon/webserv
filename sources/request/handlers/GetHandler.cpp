#include <GetHandler.hpp>

void GetHandler::executeRequest(Request& req, Response& res) {
  LOG_INFO("Processing GET request for path:", res.getReqURI());
  LOG_TRACE("GetHandler: executingRequest");
  std::shared_ptr<ProcessTreeBuilder> ptb =
    std::make_shared<ProcessTreeBuilder>(req, res, res.getServerConfig());
  //res.setReqURI(req.getReqURI());
  root = ptb->buildGetProcessTree();
  root->process(res);
}