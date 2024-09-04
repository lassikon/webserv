#include <Config.hpp>
#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <ServersManager.hpp>
#include <Signal.hpp>
#include <Utility.hpp>

sig_atomic_t g_ExitStatus;

int main(int argc, char** argv) {
  Config config;
  if (argc > 2) {
    LOG_INFO(ERR_MSG_USAGE);
    return (int)Error::Args;
  } else if (argc == 2) {
    Config config(argv[1]);
  } else {
    Config config(void);
  }
  ServersManager serversManager;
  serversManager.configServers(config);
  serversManager.runServers();
  Signal::trackSignals();
  return g_ExitStatus;
}
