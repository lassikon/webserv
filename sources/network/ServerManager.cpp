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
    pollManager.addFd(server->getSocketFd(), EPOLLIN, [&](int fd) { (void)fd; });
    LOG_DEBUG("Added server", server->getServerName(), "to pollManager");
  }
}

void ServerManager::runServers(void) {
  PollManager pollManager;
  LOG_TRACE("Adding server sockets to pollManager");
  initializePollManager(pollManager);
  while (Utility::statusOk()) {
    int epollCount = pollManager.epollWait();
    if (epollCount == -1) {
      continue;
      //throw serverError("Failed to epoll fds");
    } else if (epollCount == 0 && Utility::statusOk()) {
      handleNoEvents(pollManager);
    } else {
      serverLoop(pollManager);
    }
  }
}

void ServerManager::serverLoop(PollManager& pollManager) {
  checkChildProcesses(pollManager);
  for (auto& event : pollManager.getEpollEvents()) {
    if (event.data.fd == -1) {
      continue;
    }
    if (event.events & EPOLLERR || event.events & EPOLLHUP || event.events & EPOLLRDHUP) {
      handlePollErrors(pollManager, event);
    } else if (event.events & EPOLLIN) {
      handlePollInEvent(pollManager, event);
    } else if (event.events & EPOLLOUT) {
      handlePollOutEvent(pollManager, event);
    }
  }
  checkForNewChildProcesses(pollManager);
}

void ServerManager::handlePollErrors(PollManager& pollManager, struct epoll_event& event) {
  int fd = event.data.fd;
  int clientFd = Utility::getClientFdFromCgiParams(fd);
  if ((event.events & EPOLLHUP || event.events & EPOLLERR) && Utility::isCgiFd(fd)) {
    LOG_WARN("EPoll hangup on CGI fd:", fd);
    for (auto& server : servers) {
      if (server->isClientFd(clientFd)) {
        if (Utility::isOutReadFd(fd)) {
          server->handleClientIn(pollManager, EPOLLIN, fd, clientFd);
        } else {
          server->handleClientOut(pollManager, EPOLLOUT, fd, clientFd);
        }
        break;
      }
    }
    pollManager.removeFd(fd);
    /*     for (auto it = g_CgiParams.begin(); it != g_CgiParams.end();) {
      if (it->outReadFd == fd) {
        pollManager.removeFd(it->outReadFd);
        //pollManager.removeFd(it->inWriteFd);
        close(it->inWriteFd);
        close(it->outReadFd);
        g_CgiParams.erase(it);
        break;
      } else {
        ++it;
      }
    } */
  } else if (event.events & EPOLLERR || event.events & EPOLLRDHUP) {
    LOG_ERROR("Epoll error or hangup on fd:", fd);
    pollManager.removeFd(fd);
    /*     for (auto& server : servers) {
      if (server->isClientFd(fd)) {
        LOG_DEBUG("Removing client fd:", fd, "from server");
        server->removeClient(fd);
        break;
      }
    } */
  } else {
    LOG_WARN("EPoll on fd:", fd);
    pollManager.removeFd(fd);
  }
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
      server->handleClientIn(pollManager, event.events, fd, fd);
      break;
    } else if (Utility::isCgiFd(fd) && server->isClientFd(Utility::getClientFdFromCgiParams(fd))) {
      int clientFd = Utility::getClientFdFromCgiParams(fd);
      LOG_DEBUG("Handling CGI", fd, "EPOLLIN communication with clientFd:", clientFd);
      server->handleClientIn(pollManager, event.events, fd, clientFd);
      break;
    }
  }
}

void ServerManager::handlePollOutEvent(PollManager& pollManager, struct epoll_event& event) {
  LOG_DEBUG("Handling POLLOUT event");
  int fd = event.data.fd;
  for (auto& server : servers) {
    if (server->isClientFd(fd)) {
      LOG_TRACE("Client socket ready for writing");
      LOG_DEBUG("Handling client", fd, "POLLOUT communication");
      server->handleClientOut(pollManager, event.events, fd, fd);
      break;
    }
    if (Utility::isCgiFd(fd) && server->isClientFd(Utility::getClientFdFromCgiParams(fd))) {
      int clientFd = Utility::getClientFdFromCgiParams(fd);
      LOG_DEBUG("Handling CGI", fd, " POLLOUT communication with clientFd:", clientFd);
      server->handleClientOut(pollManager, event.events, fd, clientFd);
      break;
    }
  }
}

void ServerManager::checkForNewChildProcesses(PollManager& pollManager) {
  for (auto& cgiParam : g_CgiParams) {
    if (!pollManager.fdExists(cgiParam.outReadFd)) {
      LOG_DEBUG("Adding cgi out read fd:", cgiParam.outReadFd, " to pollManager");
      Utility::setNonBlocking(cgiParam.outReadFd);
      pollManager.addFd(cgiParam.outReadFd, EPOLLIN, [&](int fd) { close(fd); });
    }
    if (!pollManager.fdExists(cgiParam.inWriteFd)) {
      LOG_DEBUG("Adding cgi in write fd:", cgiParam.inWriteFd, " to pollManager");
      Utility::setNonBlocking(cgiParam.inWriteFd);
      pollManager.addFd(cgiParam.inWriteFd, EPOLLOUT, [&](int fd) { close(fd); });
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
  for (auto it = g_CgiParams.begin(); it != g_CgiParams.end();) {
    if (it->childExitStatus == -1) {
      pid_t result = waitpid(it->pid, &it->childExitStatus, WNOHANG);
      if (result == -1) {
        LOG_ERROR("Failed to wait for child process:", IException::expandErrno());
        pollManager.removeFd(it->outReadFd);
        pollManager.removeFd(it->inWriteFd);
        g_CgiParams.erase(it);
        // throw exception?
      } else if (result > 0) {  // Child process has exited
        LOG_INFO("Child process", it->pid, "exited with status:", it->childExitStatus);
        close(it->outWriteFd);
        close(it->inReadFd);
        it->isExited = true;
        if (it->childExitStatus != 0) {
          LOG_ERROR("Child process exited with status:", it->childExitStatus);
          it->isFailed = true;
        }
        //it = g_CgiParams.erase(it);
      } else if (result == 0 && childTimeout(it->start)) {
        LOG_ERROR("Child process", it->pid, "timed out");
        kill(it->pid, SIGKILL);
        it->isTimeout = true;
        //pollManager.removeFd(it->outReadFd);
        //pollManager.removeFd(it->inWriteFd);
        //g_CgiParams.erase(it);
      }
    }
    ++it;
  }
}
