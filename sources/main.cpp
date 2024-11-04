#include <Config.hpp>
#include <ConfigInitializer.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <RuntimeException.hpp>
#include <Server.hpp>
#include <ServerManager.hpp>
#include <SessionManager.hpp>
#include <Signal.hpp>

// POTENTIAL ISSUE: default file path allows ../../ing to access files outside of webroot
// TEST CHUNKED DATA

// check Config.hpp for http status codes

// Route Matching easy.conf
// curl --resolve other-server.com:3490:127.0.0.1 http://localhost:3490
// curl --resolve alternate-server.com:3492:127.0.0.1 http://localhost:3492
// testing modified error pages:
// curl http://localhost:3490/dfgdg

// content size limit easy.conf
// curl -X POST -H "Content-Type:text/plain" --data "THIS SHOULD NOT FIT" localhost:3490

// Testing routes to different directories
// http://localhost:3490/directory/
// http://localhost:3490/upload/

// Testing allowed methods
// curl -X POST -H "lkajsdlkj" http://localhost:3490/directory/
// curl -X DELETE http://localhost:3490/directory/default.html

// DELETE method easy.conf
// curl -X DELETE http://localhost:3490/nopermission/poke3.png
// chmod 000 webroot/website0/nopermission
// chmod 777 webroot/website0/nopermission

// Testing upload and download
// curl -o download.png http://localhost:3490/upload/poke3.png


//siege --delay=3 -c1 http://localhost:3490/cgi-bin/helloworld.cgi


// Virtual Hosts virtual.conf
// curl -H "Host: default-server.com" http://localhost:3490
// curl -H "Host: other-server.com" http://localhost:3490/
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
