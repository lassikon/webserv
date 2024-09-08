#include <ServersManager.hpp>

ServersManager::ServersManager(void) {
  LOG_DEBUG(Utility::getConstructor(*this));
}

ServersManager::~ServersManager(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}

void ServersManager::configServers(Config& config) {
  LOG_DEBUG("Initializing servers");
  servers.reserve(config.getServers().size());
  for (auto& serverConfig : config.getServers()) {
    LOG_DEBUG("Adding server", serverConfig.first);
    servers.emplace_back(serverConfig.second);
  }
  for (auto& server : servers) {
    LOG_DEBUG("Server port:", server.getPort());
  }
}

void ServersManager::runServers(void) {
  LOG_DEBUG("Running servers");
  PollManager pollManager;
  for (auto& server : servers) {
    pollManager.addFd(server.getSocketFd(), POLLIN | POLLOUT);
    LOG_DEBUG("Added server fd:", server.getSocketFd(), "port:", server.getPort(), "to pollFds");
  }
  while (true) {
    int pollCount = pollManager.pollFdsCount();
    if (pollCount == -1) {
      serverError("Failed to poll fds:");

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
    if (pollFd.revents & (POLLIN | POLLOUT)) {
      for (auto& server : servers) {
        if (pollFd.fd ==
            server.getSocketFd()) {  // It's a listening socket, accept a new connection
          server.acceptConnection(pollManager);
          break;
        } else if (server.isClientFd(
                       pollFd.fd)) {  // It's a client socket, handle client communication
          server.handleClient(pollManager, pollFd.fd, pollFd.revents);
          break;
        }
      }
    }
  }
}
