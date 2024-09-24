#pragma once

#include <Config.hpp>
#include <Logger.hpp>
#include <ProcessTree.hpp>

#include <ServeDirectoryListingAction.hpp>
#include <ServeFileAction.hpp>
#include <ServeRedirectAction.hpp>
#include <ServeQueryAction.hpp>

#include <sys/stat.h>

#include <filesystem>
#include <memory>
#include <string>

class Client;

class ProcessTreeBuilder : public std::enable_shared_from_this<ProcessTreeBuilder> {
 private:
  Client& client;
  ServerConfig& serverConfig;
  RouteConfig routeConfig;

 public:
  ProcessTreeBuilder(Client& client, ServerConfig& serverConfig);
  ~ProcessTreeBuilder() = default;
  std::shared_ptr<ProcessTree> buildGetProcessTree();
  std::shared_ptr<ProcessTree> buildPathTree();

 private:
  bool isDIrectoryListingOn(std::string& path);
  bool isIndexExist(std::string& path);
  bool isDefaultFileExist(std::string& path);
  bool isRPermOn(std::string& path);
  bool isXPermOn(std::string& path);
  bool isDirectory(std::string& path);
  bool isPathExist(std::string& path);
  bool isQuery(std::string& path);
  bool isRedirect(std::string& path);
  bool isMethodAllowed(std::string& path);
  bool isClientBodySizeAllowed(std::string& path);
  bool isRouteMatch(std::string reqURI);
  bool isErrorAsset(std::string& reqURI);
};
