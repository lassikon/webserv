#include <Client.hpp>
#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>
#include <Response.hpp>

Response::Response() {
  LOG_DEBUG(Utility::getConstructor(*this));
}

/* Response::Response(ServerConfig& serverConfig) : serverConfig(serverConfig) {
  LOG_TRACE("response constructor called");
}
 */
Response::~Response() {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}

void Response::run(std::string reqURI, std::string method, size_t bodySize) {
  LOG_TRACE("Running response for URI:", reqURI);
  std::shared_ptr<ProcessTreeBuilder> ptb = std::make_shared<ProcessTreeBuilder>(*this, serverConfig);
  //ProcessTreeBuilder ptb(*this, serverConfig);
  root = ptb->buildProcessTree();
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

std::vector<char>& Response::getResContent(void) {
  return resContent;
}

std::string& Response::getReqURI(void) {
  return reqURI;
}

std::string& Response::getReqMethod(void) {
  return reqMethod;
}

size_t& Response::getReqBodySize(void) {
  return reqBodySize;
}

RouteConfig& Response::getRouteConfig(void) {
  return routeConfig;
}

ServerConfig& Response::getServerConfig(void) {
  return serverConfig;
}

void Response::setResStatusCode(int code) {
  resStatusCode = code;
}

void Response::setResStatusMessage(std::string message) {
  resStatusMessage = message;
}

void Response::addHeader(std::string key, std::string value) {
  resHeaders[key] = value;
}

void Response::setResBody(std::vector<char>& body) {
  resBody = body;
}

void Response::setRouteConfig(RouteConfig& route) {
  routeConfig = route;
}

void Response::setServerConfig(ServerConfig server) {
  serverConfig = server;
}
