#include <Config.hpp>
#include <ServersManager.hpp>
#define PORT "3490"

// TESTING: run "nc localhost 3490" in another
// terminal (or multiple terminals to simulate multiple clients)
// and start typing messages
sig_atomic_t g_ExitStatus;

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  try {
    Config config;
    //Config config(argv[1]);
    //config.printServerConfig();
    ServersManager serversManager;
    serversManager.configServers(config);
    serversManager.runServers();
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return 0;
}
/* #include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <Server.hpp>
#include <Signal.hpp>
#include <Utility.hpp>


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
 */
