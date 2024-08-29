#include <Config.hpp>
#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <Server.hpp>
#include <Signal.hpp>
#include <Utility.hpp>

std::atomic<int> g_ExitStatus;

int main(int argc, char **argv) {
  if (argc > 2) {
    LOG_INFO(ERR_MSG_USAGE);
    return (int)Error::Args;
  } else if (argc == 2) {
    Config config(argv[1]);
  } else {
    Config config(void);
  }
  Signal::trackSignals();
  while (Utility::statusOk()) {
    LOG_INFO("Logging...");
    sleep(1);
  }
  return g_ExitStatus;
}
/*
  // Signal testing...
  while (Utility::statusOk()) {
    LOG_INFO("Logging...");
    sleep(1);
  }
*/
/*
  // Janrau's config block
  Exception::tryCatch(&Config::parseConfigFile, &config);
  if (config.getServers.empty()) {
  LOG_WARN(ERR_MSG_NOSERVER, config.getFileName);
  return (int)Error::Config;
  }
*/
/*
  // Lassi's server block
  Server server(config.getServers);
  Exception::tryCatch(&Server::Run, &server);
  return g_ExitStatus:
*/
