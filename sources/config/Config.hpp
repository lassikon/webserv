#pragma once

#include <Exception.hpp>
#include <IDirectiveSetter.hpp>
#include <Logger.hpp>
#include <Utility.hpp>

#include <map>
#include <sstream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

/*
** Config class
** configuration file can have one or multiple server blocks that starts with
** [server] block header and ends with [/server] to close the block
** each server block can have one or multiple route blocks that starts with
** [route] and ends with [/route] to close the block
*/

struct RouteConfig {
  std::string location;
  std::vector<std::string> methods;
  std::string root;
  bool directoryListing;
  std::vector<std::string> defaultFile;
  std::string uploadPath;
  std::string redirect;
  std::vector<std::string> cgi;
};

struct ServerConfig {
  std::string ipAddress;
  std::string serverName;
  int port;
  std::map<int, std::string> pagesDefault = {
      {200, "/pagesDefault/200.html"},   // ok
      {204, "/pagesDefault/204.html"},    // no content
      {301, "/pagesDefault/301.html"},   // moved permanently
      {400, "/pagesDefault/400.html"},   // bad request
      {401, "/pagesDefault/401.html"},   // unauthorized
      {403, "/pagesDefault/403.html"},   // forbidden
      {404, "/pagesDefault/404.html"},   // not found
      {405, "/pagesDefault/405.html"},   // method not allowed
      {408, "/pagesDefault/408.html"},   // request timeout
      {413, "/pagesDefault/413.html"},   // request entity too large
      {415, "/pagesDefault/415.html"},   // unsupported media type
      {500, "/pagesDefault/500.html"},   // internal server error cgi?
      {505, "/pagesDefault/505.html"},   // http version not supported
      {501, "/pagesDefault/501.html"}};  // not implemented

  std::map<int, std::string> pagesCustom;
  std::string clientBodySizeLimit;
  std::vector<RouteConfig> routes;
};

class Config {
 private:
  std::map<std::string, ServerConfig> _servers;  // ip:port as key and server config as value
  std::string _configFilePath;

 public:
  Config(std::unique_ptr<IDirectiveSetter> serverDirective,
         std::unique_ptr<IDirectiveSetter> routeDirective,
         std::string configFilePath);
  ~Config();

  void parseConfigFile();
  std::map<std::string, ServerConfig>& getServers() { return _servers; }
  void printServerConfig();
  std::string getFilePath() { return _configFilePath; }

 private:
  void closeServerBlock(std::stringstream& configFile);
  void closeRouteBlock(std::stringstream& configFile);
  void parseServerBlock(std::stringstream& configFile);
  void populateServer(ServerConfig& serverConfig, std::size_t& pos);
  void parseRouteBlock(ServerConfig& serverConfig, std::stringstream& configFile);
  void populateRoute(RouteConfig& routeConfig, std::size_t& pos);
  void addServerToMap(ServerConfig& serverConfig);
  bool callGetLine(std::stringstream& configFile);

  int getLineNumber() const { return _lineNumber; };
  void validateServer(std::map<std::string, ServerConfig>& servers);

 private:
  std::string _line;
  std::stack<std::string> _bStack;
  std::streampos _pos;
  static int _lineNumber;
  std::unique_ptr<IDirectiveSetter> _serverDirective;
  std::unique_ptr<IDirectiveSetter> _routeDirective;

 private:
  template <typename... Args> void configError(Args&&... args) {
    THROW(Error::Config, std::forward<Args>(args)...);
  }
};