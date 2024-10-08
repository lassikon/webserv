#include <Config.hpp>
#include <ConfigInitializer.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <RuntimeException.hpp>
#include <Server.hpp>
#include <ServerManager.hpp>
#include <SessionManager.hpp>
#include <Signal.hpp>

sig_atomic_t g_ExitStatus;
std::vector<struct CgiParams> g_CgiParams;

int main(int argc, char** argv) {
  (void)argv;
  if (argc > 2) {
    LOG_INFO(ERR_MSG_USAGE);
    return EXIT_FAILURE;
  } else {
    Logger::loadDefaults();
    Signal::trackSignals();
  }

  // {
  //   SessionManager s;
  //   s.setSessionCookie();
  //   s.readSessionsFromFile();
  //   s.debugPrintSessionsMap();
  //   // if (s.getSessionCookie("sessionId=QMSMFTZBHPbWLTUIODYF")) {
  //   //   LOG_INFO("Cookie found!");
  //   // } else {
  //   //   LOG_ERROR("Cookie not found!");
  //   // }
  //   // s.debugFillSessionsFile();
  //   // s.generateSessionId();
  //   // s.debugPrintSessionsMap();
  // }
  // exit(1);

  Config config = ConfigInitializer::initializeConfig(argc, argv);
  RuntimeException::tryCatch(&Config::parseConfigFile, &config);
  if (config.getServers().empty()) {
    LOG_FATAL(ERR_MSG_NOSERVER, config.getFilePath());
    return (int)RuntimeError::Config;
  }

  config.printServerConfig();
  try {
  ServerManager serverManager;
  serverManager.configServers(config);
  serverManager.runServers();}

  catch(std::exception &e) {
    std::cout << e.what();
  }

  /* ======================================================================= */
  /* ServerManager server; */
  /* Exception::tryCatch(&ServerManager::configServers, &server, config); */
  /* if (config.getServers().empty()) { */
  /*   LOG_FATAL(ERR_MSG_SERVER, config.getFileName); */
  /*   return (int)Error::Server; */
  /* } */
  /* Exception::tryCatch(&ServerManager::runServers, &server); */
  /* ======================================================================= */

  return g_ExitStatus;
}
