#pragma once

#include <Config.hpp>
#include <Logger.hpp>
#include <Utility.hpp>

#include <map>
#include <sstream>
#include <string>
#include <vector>

class Response {
 private:  // from request
  RouteConfig routeConfig;
  ServerConfig serverConfig;

  std::string reqURI;  // updated by ProcessTreeBuilder to full path

 private:  // for response
  int resStatusCode = 0;
  std::string resStatusMessage;
  std::vector<char> resBody = {};
  std::map<std::string, std::string> resHeaders = {};
  std::vector<char> resContent = {};

 public:
  Response();
  ~Response();

  void makeResponse(void);
  void makeBodytoCgi(void);

 public:  // getters
  std::vector<char>& getResContent(void) { return resContent; }
  std::vector<char>& getResBody(void) { return resBody; }
  std::string& getReqURI(void) { return reqURI; }
  RouteConfig& getRouteConfig(void) { return routeConfig; }
  ServerConfig& getServerConfig(void) { return serverConfig; }
  int getResStatusCode(void) { return resStatusCode; }
  std::map<std::string, std::string>& getResHeaders(void) { return resHeaders; }
  std::string& getResHeader(std::string key) { return resHeaders.at(key); }
  std::string& getResStatusMessage(void) { return resStatusMessage; }

 public:  // setters
  void setResStatusCode(int code) { resStatusCode = code; }
  void setResStatusMessage(std::string message) { resStatusMessage = message; }
  void addHeader(std::string key, std::string value) { resHeaders[key] = value; }
  void setReqURI(std::string uri) { reqURI = uri; }
  void setResBody(std::vector<char> body) { resBody = std::move(body); }
  void setRouteConfig(RouteConfig& route) { routeConfig = route; }
  void setServerConfig(ServerConfig server) { serverConfig = server; }
};
