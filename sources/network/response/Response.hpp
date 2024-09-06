#pragma once

#include <Config.hpp>
#include <Logger.hpp>
#include <Utility.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class Client;
class ProcessTree;
class ProcessTreeBuilder;

class Response {
 private:  // from request
  RouteConfig routeConfig;
  ServerConfig& serverConfig;
  std::string reqMethod;
  size_t reqBodySize;
  std::string reqURI;

 private:  // for response
  int resStatusCode;
  std::string resStatusMessage;
  std::vector<char> resBody = {};
  std::map<std::string, std::string> resHeaders = {};
  std::vector<char> resContent = {};
  bool transferEncodingChunked;
  std::shared_ptr<ProcessTree> root;

 public:
  Response(ServerConfig& ServerConfig);
  ~Response();

  void run(std::string reqURI, std::string method, size_t bodySize);
  void makeResponse(void);

  std::vector<char>& getResContent(void) { return resContent; }
  std::string& getReqURI(void) { return reqURI; }
  std::string& getReqMethod(void) { return reqMethod; }
  size_t& getReqBodySize(void) { return reqBodySize; }
  RouteConfig& getRouteConfig(void) { return routeConfig; }
  ServerConfig& getServerConfig(void) { return serverConfig; }
  void setResStatusCode(int code) { resStatusCode = code; }
  void setResStatusMessage(std::string message) { resStatusMessage = message; }
  void addHeader(std::string key, std::string value) {
    resHeaders[key] = value;
  }
  void setResBody(std::vector<char>& body) { resBody = body; }
  void setRouteConfig(RouteConfig& route) { routeConfig = route; }

  void printServerConfig();
};