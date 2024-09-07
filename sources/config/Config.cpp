#include "Config.hpp"

int Config::_lineNumber = 0;

Config::Config(std::unique_ptr<IDirectiveSetter> serverDirective,
               std::unique_ptr<IDirectiveSetter> routeDirective)
    : _configFilePath("confDefault/default.conf"),
      _serverDirective(std::move(serverDirective)),
      _routeDirective(std::move(routeDirective)) {
  LOG_DEBUG(Utility::getConstructor(*this));
}

Config::~Config() {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}

void Config::parseConfigFile() {
  std::stringstream configFile;
  std::vector<char> content = Utility::readFile(_configFilePath);
  if (content.empty()) {
    configError("Empty config file");
    return;
  }
  configFile.str(std::string(content.begin(), content.end()));
  while (callGetLine(configFile)) {
    if (_line.empty())
      continue;
    if (_line.compare("[server]") == 0) {
      _bStack.push("[server]");
      parseServerBlock(configFile);
      closeServerBlock(configFile);
    } else
      LOG_WARN("Parse: Invalid block,", _line, " at line", _lineNumber);
    _pos = configFile.tellg();
  }
  validateServer(_servers);
}

void Config::parseServerBlock(std::stringstream& configFile) {
  ServerConfig serverConfig;
  serverConfig = ServerConfig{};
  while (callGetLine(configFile)) {
    if (_line.empty())
      continue;
    auto delimiter_pos = _line.find(":");
    if (delimiter_pos != std::string::npos)
      populateServer(serverConfig, delimiter_pos);
    else if (_line.compare("[server]") == 0)
      return;
    else if (_line.compare("[route]") == 0) {
      _bStack.push("[route]");
      parseRouteBlock(serverConfig, configFile);
      closeRouteBlock(configFile);
    } else if (_line.compare("[/server]") == 0 &&
               (!_bStack.empty() && _bStack.top() == "[server]")) {
      _bStack.pop();
      addServerToMap(serverConfig);
      return;
    } else
      LOG_WARN("Parse: Invalid directive,", _line, " in server block at line", _lineNumber);
    _pos = configFile.tellg();
  }
}

// populate route block
void Config::parseRouteBlock(ServerConfig& serverConfig, std::stringstream& configFile) {
  RouteConfig routeConfig;
  routeConfig = RouteConfig{};
  while (callGetLine(configFile)) {
    if (_line.empty())
      continue;
    auto delimiter_pos = _line.find(":");
    if (delimiter_pos != std::string::npos)
      populateRoute(routeConfig, delimiter_pos);
    else if (_line.compare("[route]") == 0)
      return;
    else if (_line.compare("[/route]") == 0 && (!_bStack.empty() && _bStack.top() == "[route]")) {
      _bStack.pop();
      serverConfig.routes.push_back(routeConfig);
      return;
    } else
      LOG_WARN("Parse: Invalid directive, ", _line, " in route block at line ", _lineNumber);
    _pos = configFile.tellg();
  }
}

void Config::populateServer(ServerConfig& serverConfig, std::size_t& pos) {
  std::string key = _line.substr(0, pos);
  key = Utility::trimComments(key);
  key = Utility::trimWhitespaces(key);
  std::string value = _line.substr(pos + 1);
  value = Utility::trimComments(value);
  value = Utility::trimWhitespaces(value);
  _serverDirective->handleDirective(&serverConfig, key, value, _lineNumber);
}

void Config::populateRoute(RouteConfig& routeConfig, std::size_t& pos) {
  std::string key = _line.substr(0, pos);
  key = Utility::trimComments(key);
  key = Utility::trimWhitespaces(key);
  std::string value = _line.substr(pos + 1);
  value = Utility::trimComments(value);
  value = Utility::trimWhitespaces(value);
  _routeDirective->handleDirective(&routeConfig, key, value, _lineNumber);
}

// NGINX uses ip:port first, server_name second and default server last
// add server to map but with different ip, port or server name
void Config::addServerToMap(ServerConfig& serverConfig) {
  if (serverConfig.port == 0) {
    LOG_WARN("Parse: Missing port in server block at line ", _lineNumber);
    return;
  }
  std::string hostName;
  hostName = serverConfig.ipAddress + ":" + std::to_string(serverConfig.port) + " " +
             serverConfig.serverName;
  _servers.insert(std::pair<std::string, ServerConfig>(hostName, serverConfig));
}

void Config::closeServerBlock(std::stringstream& configFile) {
  if (!_bStack.empty() && _bStack.top() == "[server]") {
    LOG_WARN("Parse: Unclosed server block,", _line, " at line", _lineNumber);
    _bStack.pop();
    configFile.clear();
    configFile.seekg(_pos);
  }
}

void Config::closeRouteBlock(std::stringstream& configFile) {
  if (!_bStack.empty() && _bStack.top() == "[route]") {
    LOG_WARN("Parse: Unclosed route block,", _line, " at line", _lineNumber);
    _bStack.pop();
    configFile.clear();
    configFile.seekg(_pos);
  }
}

bool Config::callGetLine(std::stringstream& configFile) {
  // handle unexpected EOF
  if (!std::getline(configFile, _line))
    return false;
  _line = Utility::trimComments(_line);
  _line = Utility::trimWhitespaces(_line);
  _lineNumber++;
  return true;
}

void Config::validateServer(std::map<std::string, ServerConfig>& servers) {
  for (auto serverIt = servers.begin(); serverIt != servers.end(); serverIt++) {
    auto& server = serverIt->second;
    for (auto routeIt = server.routes.begin(); routeIt != server.routes.end();) {
      if (routeIt->location.empty()) {
        LOG_WARN("Parse: Missing location in route block, deleting route...");
        routeIt = server.routes.erase(routeIt);
      } else
        ++routeIt;
    }
  }
}

void Config::printServerConfig() {
  LOG_INFO("Printing server config");
  int serverCount = 1;
  for (const auto& server_pair : _servers) {
    const auto& server = server_pair.second;
    LOG_INFO("#Server: ", serverCount);
    LOG_INFO("Server host: ", server.ipAddress);
    LOG_INFO("Server port: ", server.port);
    LOG_INFO("Server name: ", server.serverName);
    LOG_INFO("Server error pages: ");
    for (const auto& error : server.pagesCustom) {
      LOG_INFO(error.first, "  path: ", error.second);
    }
    LOG_INFO("Server client body size limit: ", server.clientBodySizeLimit);
    int routeCount = 1;
    for (const auto& route : server.routes) {
      LOG_INFO("#Server: ", serverCount, " route: ", routeCount);
      LOG_INFO("  Route location: ", route.location);
      LOG_INFO("  Route methods: ");
      for (const auto& method : route.methods) {
        LOG_INFO("    ", method);
      }
      LOG_INFO("  Route cgi: ");
      for (const auto& cgi : route.cgi) {
        LOG_INFO("    ", cgi);
      }
      LOG_INFO("  Route root: ", route.root);
      LOG_INFO("  Route directory listing: ", route.directoryListing);
      for (const auto& defaultFile : route.defaultFile) {
        LOG_INFO("  Route default file: ", defaultFile);
      }
      LOG_INFO("  Route upload path: ", route.uploadPath);
      LOG_INFO("  Route redirect: ", route.redirect);
      routeCount++;
    }
    serverCount++;
  }
}