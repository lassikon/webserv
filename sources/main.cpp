#include <Config.hpp>
#include <ConfigInitializer.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <RuntimeException.hpp>
#include <Server.hpp>
#include <ServerManager.hpp>
#include <SessionManager.hpp>
#include <Signal.hpp>

std::atomic<int> g_timeOut = 10;
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
    LOG_FATAL(ERR_MSG_CONFIG, config.getFilePath());
    return (int)RuntimeError::Config;
  }
  ServerManager server;
  RuntimeException::tryCatch(&ServerManager::configServers, &server, config);
  if (server.getServers().empty()) {
    LOG_FATAL(ERR_MSG_SERVER, config.getFilePath());
    return (int)RuntimeError::Server;
  }
  RuntimeException::tryCatch(&ServerManager::runServers, &server);
  return g_ExitStatus;
}
