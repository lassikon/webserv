#include <ServerManager.hpp>

ServerManager::ServerManager(void) {
  LOG_DEBUG(Utility::getConstructor(*this));
}

ServerManager::~ServerManager(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}

bool ServerManager::checkServerExists(ServerConfig& serverConfig) {
  for (auto& server : servers) {
    if (server->getPort() == serverConfig.port) {
      return true;
    }
  }
  return false;
}

void ServerManager::configServers(Config& config) {
  LOG_TRACE("Initializing servers");
  for (auto& serverConfig : config.getServers()) {
    if (checkServerExists(serverConfig.second)) {
      LOG_DEBUG("Server already exists, adding config to its port:", serverConfig.second.port);
      for (auto& server : servers) {
        if (server->getPort() == serverConfig.second.port) {
          server->addServerConfig(serverConfig.second);
          break;
        }
      }
    } else {
      LOG_DEBUG("Creating new server config in port:", serverConfig.second.port);
      servers.emplace_back(std::make_shared<Server>(serverConfig.second));
    }
  }
}

void ServerManager::handleNoEvents(PollManager& pollManager) {
  LOG_DEBUG("No events, checking for idle clients");
  for (auto& server : servers) {
    server->checkIdleClients(pollManager);
  }
}

void ServerManager::initializePollManager(PollManager& pollManager) {
  for (auto& server : servers) {
    pollManager.addFd(server->getSocketFd(), POLLIN);
    LOG_DEBUG("Added server", server->getServerName(), "to pollFds");
  }
}

void ServerManager::runServers(void) {
  PollManager pollManager;
  LOG_TRACE("Adding server sockets to pollManager");
  initializePollManager(pollManager);
  while (true) {
    int pollCount = pollManager.pollFdsCount();
    if (pollCount == -1) {
      throw serverError("Failed to poll fds");
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
  } else if ((pollFd.revents & POLLHUP) && !isCgiFd(pollFd.fd)) {
    LOG_TRACE("Poll hangup on fd:", pollFd.fd, isCgiFd(pollFd.fd));
    LOG_WARN("Poll hangup on fd:", pollFd.fd);
    pollManager.removeFd(pollFd.fd);
    return true;
  } else if ((pollFd.revents & POLLHUP) && isCgiFd(pollFd.fd)) {
    LOG_WARN("Poll hangup on CGI fd:", pollFd.fd);
    for (auto& server : servers) {
      if (server->isClientFd(getClientFdFromCgiParams(pollFd.fd))) {
        server->handleClient(pollManager, POLLIN, pollFd.fd, getClientFdFromCgiParams(pollFd.fd));
      }
    }
    pollManager.removeFd(pollFd.fd);
    for (auto it = g_CgiParams.begin(); it != g_CgiParams.end();) {
      if (it->fd == pollFd.fd) {
        it = g_CgiParams.erase(it);
      } else {
        ++it;
      }
    }
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
    if (pollFd.fd == server->getSocketFd()) {
      LOG_TRACE("Listening socket, accept new connection");
      server->acceptConnection(pollManager);
      pollFd.revents = 0;  // Reset revents after handling
      break;
    } else if (server->isClientFd(pollFd.fd)) {
      LOG_DEBUG("Handling client", pollFd.fd, "POLLIN communication");
      server->handleClient(pollManager, pollFd.revents, pollFd.fd, pollFd.fd);
      pollFd.events &= ~POLLIN;
      pollFd.revents = 0;
      break;
    } else if (isCgiFd(pollFd.fd) && server->isClientFd(getClientFdFromCgiParams(pollFd.fd))) {
      int clientFd = getClientFdFromCgiParams(pollFd.fd);
      LOG_DEBUG("Handling CGI", pollFd.fd, " POLLIN communication with clientFd:", clientFd);
      server->handleClient(pollManager, pollFd.revents, pollFd.fd, clientFd);
      pollFd.revents = 0;
      break;
    }
  }
}

void ServerManager::handlePollOutEvent(PollManager& pollManager, struct pollfd& pollFd) {
  for (auto& server : servers) {
    if (server->isClientFd(pollFd.fd)) {
      LOG_TRACE("Client socket ready for writing");
      LOG_DEBUG("Handling client", pollFd.fd, "POLLOUT communication");
      server->handleClient(pollManager, pollFd.revents, pollFd.fd, pollFd.fd);
      if (!isCgiFd(getCgiFdFromClientFd(pollFd.fd))) {
        pollFd.events &= ~POLLOUT;
      }
      pollFd.events |= POLLIN;
      pollFd.revents = 0;
      break;
    }
  }
}

void ServerManager::checkForNewChildProcesses(PollManager& pollManager) {
  for (auto& cgiParam : g_CgiParams) {
    if (!pollManager.fdExists(cgiParam.fd)) {
      int flags = fcntl(cgiParam.fd, F_GETFL, 0);
      if (flags == -1) {
        LOG_DEBUG("Failed to get flags for pipe fd:", cgiParam.fd);
      }
      if (fcntl(cgiParam.fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_DEBUG("Failed to set pipe fd to non-blocking:", cgiParam.fd);
      }
      pollManager.addFd(cgiParam.fd, POLLIN);
      LOG_DEBUG("Added pipe fd:", cgiParam.fd, "to pollFds");
    }
  }
}

bool ServerManager::childTimeout(steady_time_point_t& start) {
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
    if (!it->isExited) {
      pid_t result = waitpid(it->pid, &status, WNOHANG);
      if (result == -1) {
        LOG_ERROR("Failed to wait for child process:", IException::expandErrno());
        it = g_CgiParams.erase(it);
        pollManager.removeFd(it->fd);
        // throw exception?
      } else if (result > 0) {  // Child process has exited
        LOG_INFO("Child process", it->pid, "exited with status:", status);
        close(it->write);
        it->isExited = true;
        //it = g_CgiParams.erase(it);
      } else if (result == 0 && childTimeout(it->start)) {
        LOG_ERROR("Child process", it->pid, "timed out");
        pollManager.removeFd(it->fd);
        it = g_CgiParams.erase(it);
      }
    }
    ++it;
  }
}

void ServerManager::serverLoop(PollManager& pollManager) {
  for (auto& pollFd : pollManager.getPollFds()) {
    if (handlePollErrors(pollManager, pollFd)) {
      continue;
    } else if (pollFd.revents & POLLIN) {
      handlePollInEvent(pollManager, pollFd);
    } else if (pollFd.revents & POLLOUT) {
      handlePollOutEvent(pollManager, pollFd);
    }
  }
  checkChildProcesses(pollManager);
  for (auto& cgi : g_CgiParams) {
    LOG_DEBUG("Cgi fd:", cgi.fd, "client fd:", cgi.clientFd);
  }
}

bool ServerManager::isCgiFd(int fd) const {
  for (auto& cgi : g_CgiParams) {
    if (cgi.fd == fd) {
      return true;
    }
  }
  return false;
}

int ServerManager::getClientFdFromCgiParams(int fd) const {
  for (auto& cgi : g_CgiParams) {
    if (cgi.fd == fd) {
      return cgi.clientFd;
    }
  }
  return -1;
}

int ServerManager::getCgiFdFromClientFd(int fd) const {
  for (auto& cgi : g_CgiParams) {
    if (cgi.clientFd == fd) {
      return cgi.fd;
    }
  }
  return -1;
}
