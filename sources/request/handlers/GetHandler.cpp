#include <GetHandler.hpp>
#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>
#include <Response.hpp>

void GetHandler::executeRequest(Request& req, Response& res) {
  LOG_TRACE("GetHandler: executingRequest");
  std::shared_ptr<ProcessTreeBuilder> ptb =
    std::make_shared<ProcessTreeBuilder>(req, res, res.getServerConfig());
  res.setReqURI(req.getReqURI());
  root = ptb->buildProcessTree();
  root->process(res);
}