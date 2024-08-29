#include <Server.hpp>

#include <Config.hpp>
#define PORT "3490"

// TESTING: run "nc localhost 3490" in another
// terminal (or multiple terminals to simulate multiple clients)
// and start typing messages

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  Config config;
  //Config config(argv[1]);
  std::shared_ptr<Config> configPtr = std::make_shared<Config>(config);
  config.printServerConfig();
  Server server(PORT, configPtr);
  server.runServer();
  return 0;
}
