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
    // if (pollFd.revents & (POLLERR || POLLHUP || POLLNVAL)) {
    //   LOG_ERROR("Error:", pollFd.revents, "on fd:", pollFd.fd);
    //   pollManager.removeFd(pollFd.fd);
    //   continue;
    // }
    if (pollFd.revents & POLLIN) {
      for (auto& server : servers) {
        if (pollFd.fd ==
            server.getSocketFd()) {  // It's a listening socket, accept a new connection
          server.acceptConnection(pollManager);
          break;
        } else if (server.isClientFd(
                       pollFd.fd)) {  // It's a client socket, handle client communication
          server.handleClient(pollManager, pollFd.fd);
          break;
        }
      }
    }
  }
}