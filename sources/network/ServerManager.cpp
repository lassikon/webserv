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
    pollManager.addFd(server->getSocketFd(), EPOLLIN);
    LOG_DEBUG("Added server", server->getServerName(), "to pollManager");
  }
}

void ServerManager::runServers(void) {
  PollManager pollManager;
  LOG_TRACE("Adding server sockets to pollManager");
  initializePollManager(pollManager);
  while (true) {
    int epollCount = pollManager.epollWait();
    if (epollCount == -1) {
      throw serverError("Failed to epoll fds");
    } else if (epollCount == 0) {
      handleNoEvents(pollManager);
    } else {
      serverLoop(pollManager);
    }
  }
}

void ServerManager::serverLoop(PollManager& pollManager) {
  for (auto& event : pollManager.getEpollEvents()) {
    if (event.events & EPOLLERR || event.events & EPOLLHUP || event.events & EPOLLRDHUP) {
      handlePollErrors(pollManager, event);
    } else if (event.events & EPOLLIN) {
      handlePollInEvent(pollManager, event);
    } else if (event.events & EPOLLOUT) {
      handlePollOutEvent(pollManager, event);
    }
  }
  checkChildProcesses(pollManager);
  for (auto& cgi : g_CgiParams) {
    LOG_DEBUG("Cgi fd:", cgi.fd, "client fd:", cgi.clientFd);
  }
}

bool ServerManager::handlePollErrors(PollManager& pollManager, struct epoll_event& event) {
  int fd = event.data.fd;
  if (event.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
    LOG_ERROR("Epoll error or hangup on fd:", fd);
    pollManager.removeFd(fd);
    return true;
  }
  return false;
}

void ServerManager::handlePollInEvent(PollManager& pollManager, struct epoll_event& event) {
  int fd = event.data.fd;
  for (auto& server : servers) {
    if (fd == server->getSocketFd()) {
      LOG_TRACE("Listening socket, accept new connection");
      server->acceptConnection(pollManager);
      break;
    } else if (server->isClientFd(fd)) {
      LOG_DEBUG("Handling client", fd, "EPOLLIN communication");
      server->handleClient(pollManager, event.events, fd, fd);
      break;
    } else if (isCgiFd(fd) && server->isClientFd(getClientFdFromCgiParams(fd))) {
      int clientFd = getClientFdFromCgiParams(fd);
      LOG_DEBUG("Handling CGI", fd, "EPOLLIN communication with clientFd:", clientFd);
      server->handleClient(pollManager, event.events, fd, clientFd);
      break;
    }
  }
}

void ServerManager::handlePollOutEvent(PollManager& pollManager, struct epoll_event& event) {
  int fd = event.data.fd;
  for (auto& server : servers) {
    if (server->isClientFd(fd)) {
      LOG_TRACE("Client socket ready for writing");
      LOG_DEBUG("Handling client", fd, "EPOLLOUT communication");
      server->handleClient(pollManager, event.events, fd, fd);
      if (!isCgiFd(getCgiFdFromClientFd(fd))) {
        pollManager.modifyFd(fd, EPOLLIN);  // Adjust fd to listen only for input
      }
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
      pollManager.addFd(cgiParam.fd, EPOLLIN);
      LOG_DEBUG("Added pipe fd:", cgiParam.fd, "to pollManager");
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
