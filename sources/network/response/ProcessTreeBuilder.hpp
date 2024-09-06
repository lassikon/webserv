#pragma once
#include <IServeAction.hpp>
#include <Logger.hpp>
#include <ProcessTree.hpp>
#include <Serve403Action.hpp>
#include <Serve404Action.hpp>
#include <Serve405Action.hpp>
#include <Serve413Action.hpp>
#include <ServeDefaultFileAction.hpp>
#include <ServeDirectoryListingAction.hpp>
#include <ServeFileAction.hpp>
#include <ServeIndexAction.hpp>
#include <ServeRedirectAction.hpp>
#include <Config.hpp>
#include <memory>
#include <string>

class Response;

class ProcessTreeBuilder {
 private:
  Response& res;
  ServerConfig& serverConfig;
  RouteConfig routeConfig;

 public:
  ProcessTreeBuilder(Response& res, ServerConfig& serverConfig);
  ~ProcessTreeBuilder() = default;
  std::shared_ptr<ProcessTree> buildProcessTree();

 private:
  bool isDIrectoryListingOn(std::string& path);
  bool isIndexExist(std::string& path);
  bool isDefaultFileExist(std::string& path);
  bool isDirectory(std::string& path);
  bool isPathExist(std::string& path);
  bool isRedirect(std::string& path);
  bool isMethodAllowed(std::string& path);
  bool isClientBodySizeAllowed(std::string& path);
  bool checkIsRouteMatch(std::string reqURI);
};