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

class Response {
 private:  // from request
  RouteConfig routeConfig;
  ServerConfig serverConfig;
  std::string reqURI; // updated by ProcessTreeBuilder to full path

 private:  // for response
  int resStatusCode;
  int cgiFd = -1;
  std::string resStatusMessage;
  std::vector<char> resBody = {};
  std::map<std::string, std::string> resHeaders = {};
  std::vector<char> resContent = {};
  bool transferEncodingChunked;

 public:
  Response();
  ~Response();

  void makeResponse(void);

  std::vector<char>& getResContent(void) { return resContent; }
  std::string& getReqURI(void) { return reqURI; }
  RouteConfig& getRouteConfig(void) { return routeConfig; }
  ServerConfig& getServerConfig(void) { return serverConfig; }
  int& getCgiFd(void) { return cgiFd; }
  void setCgiFd(int fd) { cgiFd = fd; } 
  void setResStatusCode(int code) { resStatusCode = code; }
  void setResStatusMessage(std::string message) { resStatusMessage = message; }
  void addHeader(std::string key, std::string value) { resHeaders[key] = value; }
  void setReqURI(std::string uri) { reqURI = uri; }
  void setResBody(std::vector<char>& body) { resBody = body; }
  void setRouteConfig(RouteConfig& route) { routeConfig = route; }
  void setServerConfig(ServerConfig server) { serverConfig = server; }
};