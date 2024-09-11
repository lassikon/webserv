#include <ServersManager.hpp>

ServersManager::ServersManager(void) {
  LOG_DEBUG(Utility::getConstructor(*this));
}

ServersManager::~ServersManager(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}

bool ServersManager::checkServerExists(ServerConfig& serverConfig) {
  for (auto& server : servers) {
    if (server->getPort() == serverConfig.port) {  // check ip as well?
      return true;
    }
  }
  return false;
}

void ServersManager::configServers(Config& config) {
  LOG_DEBUG("Initializing servers");
  for (auto& serverConfig : config.getServers()) {
    if (checkServerExists(serverConfig.second)) {
      LOG_DEBUG("Server already exists, adding config to existing server in port:",
                serverConfig.second.port);
      for (auto& server : servers) {
        if (server->getPort() == serverConfig.second.port) {
          server->addServerConfig(serverConfig.second);
          break;
        }
      }
    } else {
      LOG_DEBUG("Creating new server in port:", serverConfig.second.port);
      servers.emplace_back(std::make_shared<Server>(serverConfig.second));
    }
  }
}

void ServersManager::runServers(void) {
  LOG_DEBUG("Running servers");
  PollManager pollManager;
  for (auto& server : servers) {
    pollManager.addFd(server->getSocketFd(), POLLIN);
    LOG_DEBUG("Added server", server->getServerName(), "to pollFds");
  }
  while (true) {
    int pollCount = pollManager.pollFdsCount();
    if (pollCount == -1) {
      serverError("Failed to poll fds:");

    } else if (pollCount == 0) {  // No events
      LOG_DEBUG("No events, checking for idle clients");
      for (auto& server : servers) {
        server->checkIdleClients(pollManager);
      }
      continue;
    } else {
      serverLoop(pollManager);
    }
  }
}

void ServersManager::serverLoop(PollManager& pollManager) {
  for (auto& pollFd : pollManager.getPollFds()) {
    if (pollFd.revents & POLLERR) {
      LOG_ERROR("Poll error on fd:", pollFd.fd);
      pollManager.removeFd(pollFd.fd);
    } else if (pollFd.revents & POLLHUP) {
      LOG_WARN("Poll hangup on fd:", pollFd.fd);
      pollManager.removeFd(pollFd.fd);
    } else if (pollFd.revents & POLLNVAL) {
      LOG_ERROR("Invalid poll fd:", pollFd.fd);
      pollManager.removeFd(pollFd.fd);
    } else if (pollFd.revents & POLLIN) {
      for (auto& server : servers) {
        if (pollFd.fd == server->getSocketFd()) {  // Listening socket, accept a new connection
          server->acceptConnection(pollManager);
          break;
        } else if (server->isClientFd(pollFd.fd)) {  // Client socket, handle communication
          LOG_DEBUG("Handling client communication (POLLIN)");
          server->handleClient(pollManager, pollFd.fd, pollFd.revents);
          pollFd.revents = 0;  // Reset revents after handling
          break;
        }
      }
    } else if (pollFd.revents & POLLOUT) {
      for (auto& server : servers) {
        if (server->isClientFd(pollFd.fd)) {  // Client socket ready for writing
          LOG_DEBUG("Handling client communication (POLLOUT)");
          server->handleClient(pollManager, pollFd.fd, pollFd.revents);
          pollFd.revents = 0;
          break;
        }
      }
    }
  }
}
