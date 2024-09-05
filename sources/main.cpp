#include <Config.hpp>
#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <ServersManager.hpp>
#include <Signal.hpp>

sig_atomic_t g_ExitStatus;

int main(int argc, char** argv) {
  (void)argv;
  if (argc > 2) {
    LOG_INFO(ERR_MSG_USAGE);
    return (int)Error::Args;
  }

  /* ======================================================================= */
  Config config;
  /* if (argc == 2) { */
  /*   config.setFilePath(argv[1]); */
  /* } */
  Signal::trackSignals();
  /* ======================================================================= */

  //TESTING

  /* ======================================================================= */
  /* Exception::tryCatch(&Config::parseConfigFile, &config); */
  /* if (config.getServers().empty()) { */
  /*   LOG_FATAL(ERR_MSG_CONFIG, config.getFileName); */
  /*   return (int)Error::Config; */
  /* } */
  /* ServersManager server; */
  /* Exception::tryCatch(&ServersManager::configServers, &server, config); */
  /* if (config.getServers().empty()) { */
  /*   LOG_FATAL(ERR_MSG_SERVER, config.getFileName); */
  /*   return (int)Error::Server; */
  /* } */
  /* Exception::tryCatch(&ServersManager::runServers, &server); */
  /* ======================================================================= */

  return g_ExitStatus;
}
