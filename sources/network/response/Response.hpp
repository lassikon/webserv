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
  ServerConfig serverConfig;
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
  Response();
  //Response(ServerConfig& ServerConfig);
  ~Response();

  void run(std::string reqURI, std::string method, size_t bodySize);
  void makeResponse(void);

  std::vector<char>& getResContent(void);
  std::string& getReqURI(void);
  std::string& getReqMethod(void);
  size_t& getReqBodySize(void);
  RouteConfig& getRouteConfig(void);
  ServerConfig& getServerConfig(void);
  void setResStatusCode(int code);
  void setResStatusMessage(std::string message);
  void addHeader(std::string key, std::string value);
  void setResBody(std::vector<char>& body);
  void setRouteConfig(RouteConfig& route);
  void setServerConfig(ServerConfig server);
  void printServerConfig();
};