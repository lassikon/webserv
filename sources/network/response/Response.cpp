#include <Client.hpp>
#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>
#include <Response.hpp>

Response::Response() {
  LOG_TRACE("response constructor called");
}

/* Response::Response(ServerConfig& serverConfig) : serverConfig(serverConfig) {
  LOG_TRACE("response constructor called");
}
 */
Response::~Response() {
  LOG_TRACE("response destructor called");
}

void Response::run(std::string reqURI, std::string method, size_t bodySize) {
  LOG_TRACE("Running response for URI:", reqURI);
  ProcessTreeBuilder ptb(*this, serverConfig);
  root = ptb.buildProcessTree();
  reqMethod = method;
  reqBodySize = bodySize;
  this->reqURI = reqURI;
  root->process(*this);
  makeResponse();
}

void Response::makeResponse(void) {
  LOG_TRACE("Making response");
  std::ostringstream oBuf;
  oBuf << "HTTP/1.1 " << resStatusCode << " " << resStatusMessage << "\r\n";
  for (auto& [key, value] : resHeaders) {
    oBuf << key << ": " << value << "\r\n";
  }
  oBuf << "\r\n";
  std::string oBufStr = oBuf.str();
  resContent = std::vector<char>(oBufStr.begin(), oBufStr.end());
  resContent.insert(resContent.end(), resBody.begin(), resBody.end());
}
