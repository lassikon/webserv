#include <ServersManager.hpp>

ServersManager::ServersManager(void) { LOG_DEBUG("ServersManager constructor called"); }

ServersManager::~ServersManager(void) { LOG_DEBUG("ServersManager destructor called"); }

void ServersManager::runServers(void) {
  LOG_DEBUG("Running servers");
  std::vector<std::string> ports = {"3490", "3491", "3492"};
  for (auto& port : ports) {
    servers.emplace_back(port);
  }
  while (true) {
    for (auto& server : servers) {
      server.runServer();
    }
  }
}