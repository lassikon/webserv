#include <ServersManager.hpp>

ServersManager::ServersManager(void) { LOG_DEBUG("ServersManager constructor called"); }

ServersManager::~ServersManager(void) { LOG_DEBUG("ServersManager destructor called"); }

void ServersManager::runServers(void) {
  LOG_DEBUG("Running servers");

  servers.reserve(3);
  LOG_DEBUG("Adding server 3490");
  servers.emplace_back("3490");
  LOG_DEBUG("Adding server 3491");
  servers.emplace_back("3491");
  LOG_DEBUG("Adding server 3492");
  servers.emplace_back("3492");
  LOG_DEBUG("servers size:", servers.size());

  PollManager pollManager;
  for (auto& server : servers) {
    pollManager.addFd(server.getSocketFd(), POLLIN);
    LOG_DEBUG("Added server fd:", server.getSocketFd(), "port:", server.getPort(), "to pollFds");
  }
  while (true) {
    int pollCount = pollManager.pollFdsCount();
    if (pollCount == -1) {
      LOG_ERROR("Failed to poll");
      // throw exception
      break;
    } else if (pollCount == 0) {
      LOG_DEBUG("Timeout");
      continue;
    } else {
      serverLoop(pollManager);
    }
  }
}

void ServersManager::serverLoop(PollManager& pollManager) {
  for (auto& pollFd : pollManager.getPollFds()) {
    if (pollFd.revents & POLLIN) {
      for (auto& server : servers) {
        if (pollFd.fd == server.getSocketFd()) {
          server.acceptConnection(pollManager);
        } else {
          server.handleClient(pollManager, pollFd.fd);
        }
      }
    }
  }
}