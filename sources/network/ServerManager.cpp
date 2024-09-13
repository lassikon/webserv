#include <ServerManager.hpp>

ServerManager::ServerManager(void) {
  LOG_DEBUG(Utility::getConstructor(*this));
}

ServerManager::~ServerManager(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}

bool ServerManager::checkServerExists(ServerConfig& serverConfig) {
  for (auto& server : servers) {
    if (server->getPort() == serverConfig.port) {  // check ip as well?
      return true;
    }
  }
  return false;
}

void ServerManager::configServers(Config& config) {
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

void ServerManager::initializePollManager(PollManager& pollManager) {
  for (auto& server : servers) {
    pollManager.addFd(server->getSocketFd(), POLLIN);
    LOG_DEBUG("Added server", server->getServerName(), "to pollFds");
  }
}

void ServerManager::handleNoEvents(PollManager& pollManager) {
  LOG_DEBUG("No events, checking for idle clients");
  for (auto& server : servers) {
    server->checkIdleClients(pollManager);
  }
}

void ServerManager::runServers(void) {
  PollManager pollManager;
  initializePollManager(pollManager);  // Add server sockets to pollManager

  while (true) {
    int pollCount = pollManager.pollFdsCount();  // Poll for events
    if (pollCount == -1) {
      serverError("Failed to poll fds:");
    } else if (pollCount == 0) {
      handleNoEvents(pollManager);
    } else {
      serverLoop(pollManager);
    }
  }
}

bool ServerManager::handlePollErrors(PollManager& pollManager, struct pollfd& pollFd) {
  if (pollFd.revents & POLLERR) {
    LOG_ERROR("Poll error on fd:", pollFd.fd);
    pollManager.removeFd(pollFd.fd);
    return true;
  } else if (pollFd.revents & POLLHUP) {
    LOG_WARN("Poll hangup on fd:", pollFd.fd);
    pollManager.removeFd(pollFd.fd);
    return true;
  } else if (pollFd.revents & POLLNVAL) {
    LOG_ERROR("Invalid poll fd:", pollFd.fd);
    pollManager.removeFd(pollFd.fd);
    return true;
  }
  return false;
}

void ServerManager::handlePollInEvent(PollManager& pollManager, struct pollfd& pollFd) {
  for (auto& server : servers) {
    if (pollFd.fd == server->getSocketFd()) {  // Listening socket, accept a new connection
      server->acceptConnection(pollManager);
      pollFd.revents = 0;  // Reset revents after handling
      break;
    } else if (server->isClientFd(pollFd.fd)) {  // Client socket, handle communication
      LOG_DEBUG("Handling client communication (POLLIN)");
      server->handleClient(pollManager, pollFd.fd, pollFd.revents);
      pollFd.revents = 0;  // Reset revents after handling
      break;
    }
  }
}

void ServerManager::handlePollOutEvent(PollManager& pollManager, struct pollfd& pollFd) {
  for (auto& server : servers) {
    if (server->isClientFd(pollFd.fd)) {  // Client socket ready for writing
      LOG_DEBUG("Handling client communication (POLLOUT)");
      server->handleClient(pollManager, pollFd.fd, pollFd.revents);
      pollFd.revents = 0;  // Reset revents after handling
      break;
    }
  }
}

void ServerManager::checkForNewChildProcesses(PollManager& pollManager) {
  for (auto& cgiParam : g_CgiParams) {
    if (!pollManager.fdExists(cgiParam.fd)) {
      pollManager.addFd(cgiParam.fd, POLLIN);
      LOG_DEBUG("Added pipe fd:", cgiParam.fd, "to pollFds");
    }
  }
}

bool ServerManager::childTimeout(std::chrono::time_point<std::chrono::steady_clock>& start) {
  auto now = std::chrono::steady_clock::now();
  if (now - start > std::chrono::seconds(CHILD_TIMEOUT)) {
    LOG_ERROR("Child process timed out");
    return true;
  }
  return false;
}

void ServerManager::checkChildProcesses(PollManager& pollManager) {
  checkForNewChildProcesses(pollManager);
  for (auto it = g_CgiParams.begin(); it != g_CgiParams.end();) {
    int status;
    pid_t result = waitpid(it->pid, &status, WNOHANG);
    if (result == -1) {
      LOG_ERROR("Failed to wait for child process:", STRERROR);
      it = g_CgiParams.erase(it);
      pollManager.removeFd(it->fd);
      // throw exception?
    } else if (result > 0) {  // Child process has exited
      LOG_INFO("Child process", it->pid, "exited with status:", status);
      pollManager.removeFd(it->fd);
      it = g_CgiParams.erase(it);
    } else if (result == 0 && childTimeout(it->start)) {
      LOG_ERROR("Child process", it->pid, "timed out");
      pollManager.removeFd(it->fd);
      it = g_CgiParams.erase(it);
    } else {
      ++it;
    }
  }
}

void ServerManager::serverLoop(PollManager& pollManager) {
  for (auto& pollFd : pollManager.getPollFds()) {
    if (handlePollErrors(pollManager, pollFd)) {
      continue;  // Error handled, move to next pollFd
    } else if (pollFd.revents & POLLIN) {
      handlePollInEvent(pollManager, pollFd);
    } else if (pollFd.revents & POLLOUT) {
      handlePollOutEvent(pollManager, pollFd);
    }
  }
  checkChildProcesses(pollManager);
}