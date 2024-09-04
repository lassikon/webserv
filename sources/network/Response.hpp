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
  RouteConfig routeConfig;
  ServerConfig serverConfig;
  std::string reqMethod;
  int statusCode;
  std::string statusMessage;
  std::vector<char> ibody = {};  // internal body
  std::vector<char> response = {};
  std::map<std::string, std::string> headers = {};
  bool transferEncodingChunked;

 public:
  Response(ServerConfig& ServerConfig);
  ~Response();

  void run(std::string reqURI, std::string method);
  bool checkIsRouteMatch(std::string reqURI);
  std::string appendRoot(std::string reqURI);
  void makeResLine(void);
  void makeHeaders(void);
  void makeResponse(void);

  std::vector<char>& getResponse(void) { return response; }

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

  std::shared_ptr<ProcessTree> serveRedirect;
  std::shared_ptr<ProcessTree> serveDefaultFile;
  std::shared_ptr<ProcessTree> serveIndex;
  std::shared_ptr<ProcessTree> serveFile;
  std::shared_ptr<ProcessTree> serveDirectoryListing;
  std::shared_ptr<ProcessTree> serve403;
  std::shared_ptr<ProcessTree> serve404;
  std::shared_ptr<ProcessTree> serve405;

  // make decision tree
  void makeDecisionTree();
  // actions
  void serveDirectoryListingAction(std::string& path);
  void serveRedirectAction(std::string& path);
  void serveDefaultFileAction(std::string& path);
  void serveFileAction(std::string& path);
  void serveIndexAction(std::string& path);
  void serve405Action(std::string& path);
  void serve404Action(std::string& path);
  void serve403Action(std::string& path);
};