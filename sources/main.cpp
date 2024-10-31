#include <Config.hpp>
#include <ConfigInitializer.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <RuntimeException.hpp>
#include <Server.hpp>
#include <ServerManager.hpp>
#include <SessionManager.hpp>
#include <Signal.hpp>

// OPEN ISSUES
// CGI PERMISSION LOGGING

// curl http://localhost:3490/

// Virtual Hosts virtual.conf
// curl -H "Host: default-server.com" http://localhost:3490
// curl -H "Host: other-server.com" http://localhost:3490/

// Route Matching easy.conf
// curl --resolve other-server.com:3490:127.0.0.1 http://localhost:3490
// curl --resolve alternate-server.com:3492:127.0.0.1 http://localhost:3492

// content size limit easy.conf
// curl -X POST -H "Content-Type:text/plain" --data "THIS SHOULD NOT FIT" localhost:3490

// DELETE method easy.conf
// curl -X DELETE http://localhost:3490/nopermission/poke3.png
// chmod 000 webroot/website0/nopermission
// chmod 777 webroot/website0/nopermission


//siege --delay=3 -c1 http://localhost:3490/cgi-bin/helloworld.cgi

sig_atomic_t g_ExitStatus;
std::vector<struct CgiParams> g_CgiParams;

int main(int argc, char** argv) {
  if (argc > 2) {
    LOG_INFO(ERR_MSG_USAGE);
    return (int)RuntimeError::Args;
  } else {
    Logger::loadDefaults();
    Signal::trackSignals();
  }
  Config config = ConfigInitializer::initializeConfig(argc, argv);
  RuntimeException::tryCatch(&Config::parseConfigFile, &config);
  if (config.getServers().empty()) {
    LOG_FATAL(ERR_MSG_NOSERVER, config.getFilePath());
    return (int)RuntimeError::Config;
  }
  ServerManager server;
  RuntimeException::tryCatch(&ServerManager::configServers, &server, config);
  RuntimeException::tryCatch(&ServerManager::runServers, &server);
  return g_ExitStatus;
}
