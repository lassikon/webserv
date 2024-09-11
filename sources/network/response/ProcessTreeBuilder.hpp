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
#include <filesystem>
#include <sys/stat.h>

class Response;

class ProcessTreeBuilder : public std::enable_shared_from_this<ProcessTreeBuilder> {
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
  bool isRPermOn(std::string& path);
  bool isXPermOn(std::string& path);
  bool isDirectory(std::string& path);
  bool isPathExist(std::string& path);
  bool isRedirect(std::string& path);
  bool isMethodAllowed(std::string& path);
  bool isClientBodySizeAllowed(std::string& path);
  bool isRouteMatch(std::string reqURI);
  bool isErrorAsset(std::string& reqURI);
};