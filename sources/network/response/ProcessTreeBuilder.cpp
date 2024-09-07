#include <ProcessTreeBuilder.hpp>
#include <Response.hpp>

ProcessTreeBuilder::ProcessTreeBuilder(Response& res, ServerConfig& ServerConfig)
    : res(res), serverConfig(ServerConfig) {
  LOG_TRACE(Utility::getConstructor(*this));
}

std::shared_ptr<ProcessTree> ProcessTreeBuilder::buildProcessTree() {
  // ProcessTreeBuilder class for constructing the decision tree
  // Define actions
  auto serveRedirect = std::make_shared<ProcessTree>(std::make_shared<ServeRedirectAction>());
  //auto serveDefaultFile = std::make_shared<ProcessTree>(std::make_shared<ServeDefaultFileAction>());
  auto serveDirectoryListing =
    std::make_shared<ProcessTree>(std::make_shared<ServeDirectoryListingAction>());
  auto serveFile = std::make_shared<ProcessTree>(std::make_shared<ServeFileAction>());
  //auto serveIndex = std::make_shared<ProcessTree>(std::make_shared<ServeIndexAction>());
  auto serve403 = std::make_shared<ProcessTree>(std::make_shared<Serve403Action>());
  auto serve404 = std::make_shared<ProcessTree>(std::make_shared<Serve404Action>());
  auto serve405 = std::make_shared<ProcessTree>(std::make_shared<Serve405Action>());
  auto serve413 = std::make_shared<ProcessTree>(std::make_shared<Serve413Action>());

  // define process tree
  auto self = shared_from_this();
  auto isDirectoryRPermOn = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isRPermOn(path); }, serveDirectoryListing, serve403);
  auto isDirectoryListingOn = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isDIrectoryListingOn(path); }, isDirectoryRPermOn,
    serve403);
  auto isIndexRPermOn = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isRPermOn(path); }, serveFile, serve403);
  auto isIndexExist =
    std::make_shared<ProcessTree>([self](std::string& path) { return self->isIndexExist(path); },
                                  isIndexRPermOn, isDirectoryListingOn);
  auto isDefaultRPermOn = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isRPermOn(path); }, serveFile, serve403);
  auto isDefaultFileExist = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isDefaultFileExist(path); }, isDefaultRPermOn,
    isIndexExist);
  auto isFileRPermOn = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isRPermOn(path); }, serveFile, serve403);
  auto isXPermOn = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isXPermOn(path); }, isDefaultFileExist, serve403);
  auto isDirectory = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isDirectory(path); }, isXPermOn, isFileRPermOn);
  auto isPathExist = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isPathExist(path); }, isDirectory, serve404);
  auto isRedirect = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isRedirect(path); }, serveRedirect, isPathExist);
  auto isMethodAllowed = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isMethodAllowed(path); }, isRedirect, serve405);
  auto isClientBodySizeAllowed = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isClientBodySizeAllowed(path); }, isMethodAllowed,
    serve413);
  auto isRouteMatch =
    std::make_shared<ProcessTree>([self](std::string& path) { return self->isRouteMatch(path); },
                                  isClientBodySizeAllowed, serve404);
  auto isErrorAsset = std::make_shared<ProcessTree>(
    [self](std::string& path) { return self->isErrorAsset(path); }, serveFile, isRouteMatch);
  return isErrorAsset;
};

bool ProcessTreeBuilder::isDIrectoryListingOn(std::string& path) {
  LOG_TRACE("Checking directory listing on");
  (void)path;
  return res.getRouteConfig().directoryListing;
}

bool ProcessTreeBuilder::isIndexExist(std::string& path) {
  LOG_TRACE("Checking index file exist");
  if (std::filesystem ::exists(path + "index.html") ||
      std::filesystem::exists(path + "index.htm")) {
    path = path + "index.html";
    return true;
  }
  return false;
}

bool ProcessTreeBuilder::isDefaultFileExist(std::string& path) {
  LOG_TRACE("Checking default file exist");
  for (auto& defFile : res.getRouteConfig().defaultFile) {
    if (std::filesystem ::exists(path + defFile)) {
      path = path + defFile;
      return true;
    }
  }
  return false;
}

bool ProcessTreeBuilder::isXPermOn(std::string& path) {
  LOG_TRACE("Checking execute permission");
  LOG_DEBUG("Checking execute permission for path:", path);
  std::filesystem::path p(path);
  std::filesystem::file_status status = std::filesystem::status(p);
  return (
    (status.permissions() & std::filesystem::perms::owner_exec) != std::filesystem::perms::none ||
    (status.permissions() & std::filesystem::perms::group_exec) != std::filesystem::perms::none ||
    (status.permissions() & std::filesystem::perms::others_exec) != std::filesystem::perms::none);
}

bool ProcessTreeBuilder::isRPermOn(std::string& path) {
  LOG_TRACE("Checking read permission");
  LOG_DEBUG("Checking read permission for path:", path);
  std::filesystem::path p(path);
  std::filesystem::file_status status = std::filesystem::status(p);
  return (
    (status.permissions() & std::filesystem::perms::owner_read) != std::filesystem::perms::none ||
    (status.permissions() & std::filesystem::perms::group_read) != std::filesystem::perms::none ||
    (status.permissions() & std::filesystem::perms::others_read) != std::filesystem::perms::none);
}

bool ProcessTreeBuilder::isDirectory(std::string& path) {
  LOG_TRACE("Checking if path is directory");
  return std::filesystem::is_directory(path);
}

//changes reqURI to full path
bool ProcessTreeBuilder::isPathExist(std::string& path) {
  LOG_TRACE("Checking if path exists for:");
  LOG_TRACE("Requested path:", path);
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
  LOG_TRACE("Checking redirect");
  (void)path;
  return !res.getRouteConfig().redirect.empty();
}

bool ProcessTreeBuilder::isMethodAllowed(std::string& path) {
  LOG_TRACE("Checking method allowed");
  (void)path;
  auto it = std::find(res.getRouteConfig().methods.begin(), res.getRouteConfig().methods.end(),
                      res.getReqMethod());
  return it != res.getRouteConfig().methods.end();
}

bool ProcessTreeBuilder::isClientBodySizeAllowed(std::string& path) {
  LOG_TRACE("Checking client body size limit");
  (void)path;
  if (res.getReqBodySize() >
      Utility::convertSizetoBytes(res.getServerConfig().clientBodySizeLimit)) {
    return false;
  }
  return true;
}

bool ProcessTreeBuilder::isRouteMatch(std::string reqURI) {
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
    LOG_TRACE("Route found at location: ", res.getServerConfig().routes[routeIndex].location);
    res.setRouteConfig(res.getServerConfig().routes[routeIndex]);
    res.addHeader("Server", res.getServerConfig().serverName);
    return true;
  }
}

bool ProcessTreeBuilder::isErrorAsset(std::string& reqURI) {
  LOG_TRACE("Checking for error asset");
  std::filesystem::path path(reqURI);
  if (path.parent_path().string() == "/pagesDefault/assets" ||
      path.parent_path().string() == "/pagesCustom/assets") {
    std::filesystem::path exePath;
    exePath = Utility::getExePath(exePath);
    reqURI = reqURI.substr(1, reqURI.size());
    std::filesystem::path errorPath = exePath / reqURI;
    reqURI = errorPath.string();
    return true;
  }
  return false;
}