#include <Client.hpp>
#include <Response.hpp>

Response::Response(ServerConfig& serverConfig) : serverConfig(serverConfig) {
  LOG_TRACE("response constructor called");
  makeDecisionTree();
}

Response::~Response() { LOG_TRACE("response destructor called"); }

void Response::makeDecisionTree() {
  LOG_TRACE("Making decision tree");
  // terminal nodes / actions
  serveRedirect = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serveRedirectAction(path); });
  serveDirectoryListing = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serveDirectoryListingAction(path); });
  serveDefaultFile = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serveDefaultFileAction(path); });
  serveFile = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serveFileAction(path); });
  serveIndex = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serveIndexAction(path); });
  serve403 = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serve403Action(path); });
  serve404 = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serve404Action(path); });
  serve405 = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serve405Action(path); });
  serve413 = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serve413Action(path); });
  // define process tree

  isDirectoryListingOn = std::make_shared<ProcessTree>(
      [this](std::string& path) {
        (void)path;
        LOG_DEBUG("isDirectoryListingOn: ", routeConfig.directoryListing);
        return routeConfig.directoryListing;
      },
      serveDirectoryListing, serve403);

  isIndexExist = std::make_shared<ProcessTree>(
      [this](std::string& path) {
        LOG_DEBUG("isIndexExist: ", path);
        return std::filesystem::exists(path + "index.html") ||
               std::filesystem::exists(path + "index.htm");
      },
      serveIndex, isDirectoryListingOn);

  isDefaultFileExist = std::make_shared<ProcessTree>(
      [this](std::string& path) {
        for (auto& defFile : routeConfig.defaultFile) {
          if (std::filesystem::exists(path + defFile)) {
            LOG_DEBUG("isDefaultFileExist: ", path + defFile);
            return true;
          }
        };
        return false;
      },
      serveDefaultFile, isIndexExist);

  isDirectory = std::make_shared<ProcessTree>(
      [this](std::string& path) {
        LOG_DEBUG("isDirectory: ", path);
        return std::filesystem::is_directory(path);
      },
      isDefaultFileExist, serveFile);

  isPathExist = std::make_shared<ProcessTree>(
      [this](std::string& path) {
        LOG_DEBUG("isPathExist: ", path);
        path = appendRoot(path);
        return std::filesystem::exists(path);
      },
      isDirectory, serve404);
  isRedirect = std::make_shared<ProcessTree>(
      [this](std::string& path) {
        (void)path;
        LOG_DEBUG("isRedirect: ", routeConfig.redirect);
        return !routeConfig.redirect.empty();
      },
      serveRedirect, isPathExist);
  isMethodAllowed = std::make_shared<ProcessTree>(
      [this](std::string& path) {
        (void)path;
        auto it = std::find(routeConfig.methods.begin(),
                            routeConfig.methods.end(), reqMethod);
        LOG_DEBUG("isMethodAllowed: ", reqMethod);
        return it != routeConfig.methods.end();
      },
      isRedirect, serve405);

  isClientBodySizeAllowed = std::make_shared<ProcessTree>(
      [this](std::string& path) {
        (void)path;

        if (reqBodySize > Utility::convertSizetoBytes(serverConfig.clientBodySizeLimit)) {
          LOG_WARN("Client body size limit exceeded");
          return false;
        }
        return true;
      },
      isMethodAllowed, serve413);

  isRouteMatch = std::make_shared<ProcessTree>(
    [this](std::string& path) {
      return checkIsRouteMatch(path);
    }, isClientBodySizeAllowed, serve404);

  root = isRouteMatch;
}

void Response::run(std::string reqURI, std::string method, size_t bodySize) {
  LOG_TRACE("Running response", reqURI);
  reqMethod = method;
  reqBodySize = bodySize;
  root->process(reqURI);
  makeResponse();
}

bool Response::checkIsRouteMatch(std::string reqURI) {
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
    LOG_WARN("No matching route found");
    LOG_DEBUG("Request URI:", reqURI);
    headers["Server"] = serverConfig.serverName;
    return false;
  } else {  // found matching route
    LOG_DEBUG("Route found: ", serverConfig.routes[routeIndex].location);
    routeConfig = serverConfig.routes[routeIndex];
    headers["Server"] = serverConfig.serverName;
    return true;
  }
}

std::string Response::appendRoot(std::string reqURI) {
  LOG_TRACE("Searching for request");
  if (reqURI.front() == '/') {
    if (reqURI.size() > 1) {
      reqURI = reqURI.substr(1, reqURI.size());
    } else {
      reqURI = "";
    }
  }
  std::filesystem::path rootPath = routeConfig.root;
  std::filesystem::path path = rootPath / reqURI;
  LOG_DEBUG("Path:", path, "Request URI:", reqURI);
  std::string pathStr = path.string();
  return pathStr;
}


void Response::serveRedirectAction(std::string& path){
  LOG_TRACE("Checking if redirect");
  (void)path;
  LOG_DEBUG("Redirecting to: ", routeConfig.redirect);
  statusCode = 301;
  statusMessage = "Moved Permanently";
  headers["Location"] = routeConfig.redirect;
  headers["Connection"] = "close";
}

void Response::serveDirectoryListingAction(std::string& path) {
  LOG_TRACE("Serving directory listing");
  (void)path;
}

void Response::serveDefaultFileAction(std::string& path) {
  LOG_TRACE("Serving default file");
  statusCode = 200;
  statusMessage = "OK";
  auto it =
      std::find_if(routeConfig.defaultFile.begin(),
                   routeConfig.defaultFile.end(), [path](std::string& defFile) {
                     return std::filesystem::exists(path + defFile);
                   });
  path += *it;
  ibody = Utility::readFile(path);
  std::string ext = path.substr(path.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  headers["Content-Type"] = mimeType;
  headers["Content-Length"] = std::to_string(ibody.size());
  headers["Connection"] = "close";
}

void Response::serveFileAction(std::string& path) {
  LOG_TRACE("Serving file");
  statusCode = 200;
  statusMessage = "OK";
  ibody = Utility::readFile(path);
  std::string ext = path.substr(path.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  headers["Content-Type"] = mimeType;
  headers["Content-Length"] = std::to_string(ibody.size());
  headers["Connection"] = "close";
}

void Response::serveIndexAction(std::string& path) {
  LOG_TRACE("Serving index");
  statusCode = 200;
  statusMessage = "OK";
  auto it =
      std::find_if(routeConfig.defaultFile.begin(),
                   routeConfig.defaultFile.end(), [path](std::string& defFile) {
                     return std::filesystem::exists(path + defFile);
                   });
  path += *it;
  ibody = Utility::readFile(path);
  std::string ext = path.substr(path.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  headers["Content-Type"] = mimeType;
  headers["Content-Length"] = std::to_string(ibody.size());
  headers["Connection"] = "close";
}

void Response::serve403Action(std::string& path) {
  LOG_TRACE("Serving error 403");
  statusCode = 403;
  statusMessage = "Forbidden";
  auto key = serverConfig.pagesDefault.find(403);
  path = key->second;
  std::filesystem::path exePath;
  exePath = Utility::getExePath(exePath);
  if (path.front() == '/') {
    path = path.substr(1, path.size());
  }
  std::filesystem::path errorPath = exePath / path;
  std::string errorPathStr = errorPath.string();
  ibody = Utility::readFile(errorPathStr);
  std::string ext = errorPathStr.substr(errorPathStr.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  headers["Content-Type"] = mimeType;
  headers["Content-Length"] = std::to_string(ibody.size());
  headers["Connection"] = "close";
}

void Response::serve404Action(std::string& path) {
  LOG_TRACE("Serving error 404");
  LOG_DEBUG("Forbidden path:", path);
  statusCode = 404;
  statusMessage = "Not Found";
  if (path.compare(0, 20, "/pagesDefault/assets") != 0) {
    auto key = serverConfig.pagesDefault.find(404);
    path = key->second;
  }
  std::filesystem::path exePath;
  exePath = Utility::getExePath(exePath);
  if (path.front() == '/') {
    path = path.substr(1, path.size());
  }
  std::filesystem::path errorPath = exePath / path;
  std::string errorPathStr = errorPath.string();
  LOG_DEBUG("Error path:", errorPathStr);
  ibody = Utility::readFile(errorPathStr);
  std::string ext = errorPathStr.substr(errorPathStr.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  headers["Content-Type"] = mimeType;
  headers["Content-Length"] = std::to_string(ibody.size());
  headers["Connection"] = "close";
}

void Response::serve405Action(std::string& path) {
  LOG_TRACE("Serving error 405");
  statusCode = 405;
  statusMessage = "Method Not Allowed";
  auto key = serverConfig.pagesDefault.find(405);
  path = key->second;
  std::filesystem::path exePath;
  exePath = Utility::getExePath(exePath);
  if (path.front() == '/') {
    path = path.substr(1, path.size());
  }
  std::filesystem::path errorPath = exePath / path;
  std::string errorPathStr = errorPath.string();
  ibody = Utility::readFile(errorPathStr);
  std::string ext = errorPathStr.substr(errorPathStr.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  headers["Content-Type"] = mimeType;
  headers["Content-Length"] = std::to_string(ibody.size());
  headers["Connection"] = "close";
}

void Response::serve413Action(std::string& path) {
  LOG_TRACE("Serving error 413");
  statusCode = 413;
  statusMessage = "Payload Too Large";
  auto key = serverConfig.pagesDefault.find(413);
  path = key->second;
  std::filesystem::path exePath;
  exePath = Utility::getExePath(exePath);
  if (path.front() == '/') {
    path = path.substr(1, path.size());
  }
  std::filesystem::path errorPath = exePath / path;
  std::string errorPathStr = errorPath.string();
  ibody = Utility::readFile(errorPathStr);
  std::string ext = errorPathStr.substr(errorPathStr.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  headers["Content-Type"] = mimeType;
  headers["Content-Length"] = std::to_string(ibody.size());
  headers["Connection"] = "close";
}


void Response::makeResponse(void) {
  LOG_TRACE("Making response");
  std::ostringstream oBuf;
  oBuf << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
  for (auto& [key, value] : headers) {
    oBuf << key << ": " << value << "\r\n";
  }
  oBuf << "\r\n";
  std::string oBufStr = oBuf.str();
  response = std::vector<char>(oBufStr.begin(), oBufStr.end());
  response.insert(response.end(), ibody.begin(), ibody.end());
}
