#include <DeleteHandler.hpp>

void DeleteHandler::executeRequest(Request& req, Response& res) {
  LOG_INFO("Processing DELETE request for path:", req.getReqURI());
  LOG_TRACE("DeleteHandler: executingRequest");
  (void)req;
    (void)res;
}