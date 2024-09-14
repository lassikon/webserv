#include <ServersManager.hpp>

ServersManager::ServersManager(void) {
  LOG_TRACE(Utility::getConstructor(*this));
}

ServersManager::~ServersManager(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
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
  LOG_TRACE("Initializing servers");
  for (auto& serverConfig : config.getServers()) {
    if (checkServerExists(serverConfig.second)) {
      LOG_DEBUG("Adding config to existing server in port:", serverConfig.second.port);
      for (auto& server : servers) {
        if (server->getPort() == serverConfig.second.port) {
          server->addServerConfig(serverConfig.second);
          break;
        }
      }
    } else {
      LOG_DEBUG("Creating config to new server in port:", serverConfig.second.port);
      servers.emplace_back(std::make_shared<Server>(serverConfig.second));
    }
  }
}

void ServersManager::runServers(void) {
  LOG_TRACE("Running servers");
  PollManager pollManager;
  for (auto& server : servers) {
    pollManager.addFd(server->getSocketFd(), POLLIN | POLLOUT);
    LOG_DEBUG("Added server", server->getServerName(), "to pollFds");
  }
  while (true) {
    int pollCount = pollManager.pollFdsCount();
    if (pollCount == -1) {
      throw serverError("Failed to poll fds");
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
        if (pollFd.fd == server->getSocketFd()) {
          LOG_DEBUG("Listening socket, accept a new connection");
          server->acceptConnection(pollManager);
          break;
        } else if (server->isClientFd(pollFd.fd)) {
          LOG_DEBUG("Client socket, handle client communication");
          server->handleClient(pollManager, pollFd.fd, pollFd.revents);
          break;
        }
      }
    }
  }
}
