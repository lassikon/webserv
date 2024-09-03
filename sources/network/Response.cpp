#include <Client.hpp>
#include <Response.hpp>

Response::Response(ServerConfig& serverConfig) : serverConfig(serverConfig) {
  LOG_TRACE("response constructor called");
  makeDecisionTree();
}

Response::~Response() { LOG_TRACE("response destructor called"); }

void Response::run(Client* client, std::string reqURI, std::string method) {
  LOG_TRACE("Running response", reqURI);
  if (!matchRoute(reqURI)) return;
  if (!isMethodAllowed(method)) return;
  if (isRedirect(reqURI)) {
    sendResponse(client->getFd());
    return;
  }
  searchRequest(reqURI);
  makeResLine();
  makeHeaders(finalPath);
  makeBody();
  sendResponse(client->getFd());
}

bool Response::matchRoute(std::string reqURI) {
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
    LOG_WARN("No matching route found", reqURI);
    serve404Action(reqURI);
    return false;
  } else {  // found matching route
    LOG_DEBUG("Route found: ", serverConfig.routes[routeIndex].location);
    routeConfig = serverConfig.routes[routeIndex];
    return true;
  }
}

bool Response::isMethodAllowed(std::string method) {
  LOG_TRACE("Checking if method is allowed");
  auto it =
      std::find(routeConfig.methods.begin(), routeConfig.methods.end(), method);
  if (it == routeConfig.methods.end()) {
    LOG_WARN("Method not allowed");
    serve405Action();
    return false;
  }
  return true;
}

bool Response::isRedirect(std::string reqURI){
  LOG_TRACE("Checking if redirect");
  (void)reqURI;
  std::cout << "redirect: " << routeConfig.redirect << std::endl;
  if (!routeConfig.redirect.empty()) {
    LOG_DEBUG("Redirecting to: ", routeConfig.redirect);
    statusCode = 301;
    statusMessage = "Moved Permanently";
    makeResLine();
    std::ostringstream oBuf;
    oBuf << "Server: " << serverConfig.serverName << "\r\n";
    oBuf << "Location: " << routeConfig.redirect << "\r\n";
    oBuf << "Connection: close\r\n";
    oBuf << "\r\n";
    std::string oBufStr = oBuf.str();
    headers = std::vector<char>(oBufStr.begin(), oBufStr.end());
    makeBody();
    return true;
  }
  return false;
}

bool Response::searchRequest(std::string reqURI) {
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
  root->process(pathStr);
  return true;
}

void Response::makeDecisionTree() {
  LOG_TRACE("Making decision tree");
  // terminal nodes / actions
  serveDirectoryListing = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serveDirectoryListingAction(path); });
  serveDefaultFile = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serveDefaultFileAction(path); });
  serveFile = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serveFileAction(path); });
  serveIndex = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serveIndexAction(path); });
  serve404 = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serve404Action(path); });
  serve403 = std::make_shared<ProcessTree>(
      [this](std::string& path) { this->serve403Action(path); });
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
        return std::filesystem::exists(path);
      },
      isDirectory, serve404);

  root = isPathExist;
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
  finalPath = path;
}

void Response::serveFileAction(std::string& path) {
  LOG_TRACE("Serving file");
  statusCode = 200;
  statusMessage = "OK";
  ibody = Utility::readFile(path);
  finalPath = path;
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
  finalPath = path;
}

void Response::serve404Action(std::string& path) {
  LOG_TRACE("Serving error");
  statusCode = 404;
  statusMessage = "Not Found";
  auto key = serverConfig.pagesDefault.find(404);
  path = key->second;
  std::filesystem::path exePath;
  exePath = Utility::getExePath(exePath);
  if (path.front() == '/') {
    path = path.substr(1, path.size());
  }
  std::filesystem::path errorPath = exePath / path;
  std::string errorPathStr = errorPath.string();
  ibody = Utility::readFile(errorPathStr);
  finalPath = errorPathStr;
}

void Response::serve403Action(std::string& path) {
  LOG_TRACE("Serving error");
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
  finalPath = errorPathStr;
}

void Response::serve405Action(void) {
  LOG_TRACE("Serving error");
  statusCode = 405;
  statusMessage = "Method Not Allowed";
  auto key = serverConfig.pagesDefault.find(405);
  std::string path = key->second;
  std::filesystem::path exePath;
  exePath = Utility::getExePath(exePath);
  if (path.front() == '/') {
    path = path.substr(1, path.size());
  }
  std::filesystem::path errorPath = exePath / path;
  std::string errorPathStr = errorPath.string();
  ibody = Utility::readFile(errorPathStr);
  finalPath = errorPathStr;
}

void Response::makeResLine(void) {
  LOG_TRACE("Making response line");
  std::ostringstream oBuf;
  oBuf << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
  std::string oBufStr = oBuf.str();
  responseLine = std::vector<char>(oBufStr.begin(), oBufStr.end());
}

void Response::makeHeaders(std::string& extension) {
  LOG_TRACE("Making headers");
  std::string ext = extension.substr(extension.find_last_of(".") + 1);
  std::string mimeType = Utility::getMimeType(ext);
  std::ostringstream oBuf;
  oBuf << "Server: " << serverConfig.serverName << "\r\n";
  oBuf << "Content-Type: " << mimeType << "\r\n";
  oBuf << "Content-Length: " << ibody.size() << "\r\n";
  oBuf << "Connection: close\r\n";
  oBuf << "\r\n";
  std::string oBufStr = oBuf.str();
  headers = std::vector<char>(oBufStr.begin(), oBufStr.end());
}

void Response::makeBody(void) {
  LOG_TRACE("Making body");
  response = responseLine;
  response.insert(response.end(), headers.begin(), headers.end());
  response.insert(response.end(), ibody.begin(), ibody.end());
}

void Response::sendResponse(int clientFd) {
  LOG_TRACE("Sending response");
  if (send(clientFd, response.data(), response.size(), 0) == -1) {
    LOG_ERROR("Failed to send response");
  }
}