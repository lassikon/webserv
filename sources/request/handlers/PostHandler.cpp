#include <PostHandler.hpp>

void PostHandler::executeRequest(Request& req, Response& res) {
  LOG_TRACE("PostHandler: executingRequest");
  LOG_INFO("Processing POST request for path:", req.getReqURI());
  (void)req;
    (void)res;
}