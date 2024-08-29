#include <Config.hpp>
#include <ServersManager.hpp>
#define PORT "3490"

// TESTING: run "nc localhost 3490" in another
// terminal (or multiple terminals to simulate multiple clients)
// and start typing messages

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  try {
    // Config config(argv[1]);
    Config config;
    config.printServerConfig();
    ServersManager serversManager;
    serversManager.runServers();
  } catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return EXIT_FAILURE;
  }
  return 0;
}
