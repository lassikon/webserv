#pragma once

#include <Config.hpp>
#include <Logger.hpp>
#include <ProcessTree.hpp>
#include <Utility.hpp>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

class Client;

class Response {
 private:
  const ServerConfig& serverConfig;
  std::string reqMethod;
  size_t reqBodySize;
  std::string reqURI;

 private:
  RouteConfig routeConfig;
  int resStatusCode;
  std::string resStatusMessage;
  std::vector<char> resBody = {};  // internal body
  std::vector<char> resContent = {};
  std::map<std::string, std::string> resHeaders = {};
  bool transferEncodingChunked;

 public:
  Response(const ServerConfig& ServerConfig);
  ~Response();

  void run(std::string reqURI, std::string method, size_t bodySize);
  bool checkIsRouteMatch(std::string reqURI);
  std::string appendRoot(std::string reqURI);
  void makeResLine(void);
  void makeHeaders(void);
  void makeResponse(void);

  std::vector<char>& getResContent(void) { return resContent; }

 private:
  std::shared_ptr<ProcessTree> root;
  std::shared_ptr<ProcessTree> isRouteMatch;
  std::shared_ptr<ProcessTree> isMethodAllowed;
  std::shared_ptr<ProcessTree> isRedirect;
  std::shared_ptr<ProcessTree> isPathExist;
  std::shared_ptr<ProcessTree> isDirectory;
  std::shared_ptr<ProcessTree> isFileExist;
  std::shared_ptr<ProcessTree> isDefaultFileExist;
  std::shared_ptr<ProcessTree> isIndexExist;
  std::shared_ptr<ProcessTree> isDirectoryListingOn;
  std::shared_ptr<ProcessTree> isClientBodySizeAllowed;

  std::shared_ptr<ProcessTree> serveRedirect;
  std::shared_ptr<ProcessTree> serveDefaultFile;
  std::shared_ptr<ProcessTree> serveIndex;
  std::shared_ptr<ProcessTree> serveFile;
  std::shared_ptr<ProcessTree> serveDirectoryListing;
  std::shared_ptr<ProcessTree> serve403;
  std::shared_ptr<ProcessTree> serve404;
  std::shared_ptr<ProcessTree> serve405;
  std::shared_ptr<ProcessTree> serve413;

  // make decision tree
  void makeDecisionTree();
  // actions
  void serveDirectoryListingAction(std::string& path);
  void serveRedirectAction(std::string& path);
  void serveDefaultFileAction(std::string& path);
  void serveFileAction(std::string& path);
  void serveIndexAction(std::string& path);
  void serve403Action(std::string& path);
  void serve404Action(std::string& path);
  void serve405Action(std::string& path);
  void serve413Action(std::string& path);
};