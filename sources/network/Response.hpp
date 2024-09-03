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
  int statusCode;
  std::string statusMessage;
  std::vector<char> responseLine = {};
  std::vector<char> ibody = {};  // internal body
  std::vector<char> headers = {};
  std::vector<char> response = {};
  std::string finalPath;
  bool transferEncodingChunked;

 public:
  Response(ServerConfig& ServerConfig);
  ~Response();

  void run(Client* client, std::string reqURI, std::string method);
  bool matchRoute(std::string reqURI);
  bool isMethodAllowed(std::string method);
  bool isRedirect(std::string reqURI);
  bool searchRequest(std::string reqURI);
  void makeResLine(void);
  void makeHeaders(std::string& extension);
  void makeBody(void);
  void sendResponse(int clientFd);

 private:
  std::shared_ptr<ProcessTree> root;
  std::shared_ptr<ProcessTree> isPathExist;
  std::shared_ptr<ProcessTree> isDirectory;
  std::shared_ptr<ProcessTree> isFileExist;
  std::shared_ptr<ProcessTree> isDefaultFileExist;
  std::shared_ptr<ProcessTree> isIndexExist;
  std::shared_ptr<ProcessTree> isDirectoryListingOn;

  std::shared_ptr<ProcessTree> serveDefaultFile;
  std::shared_ptr<ProcessTree> serveIndex;
  std::shared_ptr<ProcessTree> serveFile;
  std::shared_ptr<ProcessTree> serveDirectoryListing;
  std::shared_ptr<ProcessTree> serve404;
  std::shared_ptr<ProcessTree> serve403;

  // make decision tree
  void makeDecisionTree();
  // actions
  void serveDirectoryListingAction(std::string& path);
  void serveDefaultFileAction(std::string& path);
  void serveFileAction(std::string& path);
  void serveIndexAction(std::string& path);
  void serve405Action(void);
  void serve404Action(std::string& path);
  void serve403Action(std::string& path);
};