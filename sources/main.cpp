#include <Config.hpp>
#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <ServersManager.hpp>
#include <Signal.hpp>

sig_atomic_t g_ExitStatus;

int main(int argc, char** argv) {
  (void)argc;
  try {
    Signal::trackSignals();
    Config config(argv[1]);
    config.printServerConfig();
    ServersManager serversManager;
    serversManager.configServers(config);
    // serversManager.runServers();
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return g_ExitStatus;
  }
  return g_ExitStatus;
}
