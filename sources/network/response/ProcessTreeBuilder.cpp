#include <ProcessTreeBuilder.hpp>
#include <Response.hpp>

ProcessTreeBuilder::ProcessTreeBuilder(Response& res,
                                       ServerConfig& ServerConfig)
    : res(res), serverConfig(ServerConfig) {
  LOG_TRACE("ProcessTreeBuilder constructor called");
}

std::shared_ptr<ProcessTree> ProcessTreeBuilder::buildProcessTree() {
  // ProcessTreeBuilder class for constructing the decision tree
  // Define actions
  auto serveRedirect =
      std::make_shared<ProcessTree>(std::make_shared<ServeRedirectAction>());
  auto serveDefaultFile =
      std::make_shared<ProcessTree>(std::make_shared<ServeDefaultFileAction>());
  auto serveDirectoryListing = std::make_shared<ProcessTree>(
      std::make_shared<ServeDirectoryListingAction>());
  auto serveFile =
      std::make_shared<ProcessTree>(std::make_shared<ServeFileAction>());
  auto serveIndex =
      std::make_shared<ProcessTree>(std::make_shared<ServeIndexAction>());
  auto serve403 =
      std::make_shared<ProcessTree>(std::make_shared<Serve403Action>());
  auto serve404 =
      std::make_shared<ProcessTree>(std::make_shared<Serve404Action>());
  auto serve405 =
      std::make_shared<ProcessTree>(std::make_shared<Serve405Action>());
  auto serve413 =
      std::make_shared<ProcessTree>(std::make_shared<Serve413Action>());

  // define process tree
  auto isDirectoryListingOn = std::make_shared<ProcessTree>(
      [this](std::string& path) { return this->isDIrectoryListingOn(path); },
      serveDirectoryListing, serve403);
  auto isIndexExist = std::make_shared<ProcessTree>(
      [this](std::string& path) { return this->isIndexExist(path); },
      serveIndex, isDirectoryListingOn);
  auto isDefaultFileExist = std::make_shared<ProcessTree>(
      [this](std::string& path) { return this->isDefaultFileExist(path); },
      serveDefaultFile, isIndexExist);
  auto isDirectory = std::make_shared<ProcessTree>(
      [this](std::string& path) { return this->isDirectory(path); },
      isDefaultFileExist, serveFile);
  auto isPathExist = std::make_shared<ProcessTree>(
      [this](std::string& path) { return this->isPathExist(path); },
      isDirectory, serve404);
  auto isRedirect = std::make_shared<ProcessTree>(
      [this](std::string& path) { return this->isRedirect(path); },
      serveRedirect, isPathExist);
  auto isMethodAllowed = std::make_shared<ProcessTree>(
      [this](std::string& path) { return this->isMethodAllowed(path); },
      isRedirect, serve405);
  auto isClientBodySizeAllowed = std::make_shared<ProcessTree>(
      [this](std::string& path) { return this->isClientBodySizeAllowed(path); },
      isMethodAllowed, serve413);
  auto isRouteMatch = std::make_shared<ProcessTree>(
      [this](std::string& path) { return checkIsRouteMatch(path); },
      isClientBodySizeAllowed, serve404);
  return isRouteMatch;
};

bool ProcessTreeBuilder::isDIrectoryListingOn(std::string& path) {
  LOG_DEBUG("Checking directory listing on");
  (void)path;
  return res.getRouteConfig().directoryListing;
}

bool ProcessTreeBuilder::isIndexExist(std::string& path) {
  LOG_DEBUG("Checking index file exist");
  return std::filesystem ::exists(path + "index.html") ||
         std::filesystem::exists(path + "index.htm");
}

bool ProcessTreeBuilder::isDefaultFileExist(std::string& path) {
  LOG_DEBUG("Checking default file exist");
  for (auto& defFile : res.getRouteConfig().defaultFile) {
    if (std::filesystem ::exists(path + defFile)) {
      return true;
    }
  }
  return false;
}

bool ProcessTreeBuilder::isDirectory(std::string& path) {
  LOG_INFO("Checking if path is directory", path);
  LOG_DEBUG("Checking if path is directory");
  return std::filesystem::is_directory(path);
}

//changes reqURI to full path
bool ProcessTreeBuilder::isPathExist(std::string& path) {
  LOG_DEBUG("Checking if path exists for:");
  LOG_DEBUG("Requested path:", path);
  if (path.front() == '/') {
    if (path.size() > 1) {
      path = path.substr(1, path.size());
    } else {
      path = "";
    }
  }
  std::filesystem::path rootPath = res.getRouteConfig().root;
  std::filesystem::path fullpath = rootPath / path;
  path = fullpath.string();
  return std::filesystem::exists(path);
}

bool ProcessTreeBuilder::isRedirect(std::string& path) {
  LOG_DEBUG("Checking redirect");
  (void)path;
  return !res.getRouteConfig().redirect.empty();
}

bool ProcessTreeBuilder::isMethodAllowed(std::string& path) {
  LOG_DEBUG("Checking method allowed");
  (void)path;
  auto it = std::find(res.getRouteConfig().methods.begin(),
                      res.getRouteConfig().methods.end(), res.getReqMethod());
  return it != res.getRouteConfig().methods.end();
}

bool ProcessTreeBuilder::isClientBodySizeAllowed(std::string& path) {
  LOG_DEBUG("Checking client body size limit");
  (void)path;
  if (res.getReqBodySize() >
      Utility::convertSizetoBytes(res.getServerConfig().clientBodySizeLimit)) {
    return false;
  }
  return true;
}

bool ProcessTreeBuilder::checkIsRouteMatch(std::string reqURI) {
  LOG_TRACE("Searching for matching route");
  size_t routeIt;
  size_t maxMatchingLen = 0;
  size_t routeIndex = 0;
  for (routeIt = 0; routeIt < serverConfig.routes.size(); routeIt++) {
    size_t rLocLen = serverConfig.routes[routeIt].location.length();
    std::string rLoc = serverConfig.routes[routeIt].location;
    if (reqURI.compare(0, rLocLen, rLoc) == 0) {
      if (rLocLen > maxMatchingLen) {
        maxMatchingLen = rLocLen;
        routeIndex = routeIt;
      }
    }
  }
  if (maxMatchingLen == 0) {
    LOG_WARN("No matching route found for URI:", reqURI);
    res.addHeader("Server", res.getServerConfig().serverName);
    return false;
  } else {
    LOG_DEBUG("Route found at location: ",
              res.getServerConfig().routes[routeIndex].location);
    res.setRouteConfig(res.getServerConfig().routes[routeIndex]);
    res.addHeader("Server", res.getServerConfig().serverName);
    return true;
  }
}