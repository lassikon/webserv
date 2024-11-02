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
      LOG_DEBUG("Server already exists, adding config to its port:",
                serverConfig.second.port);
      for (auto& server : servers) {
        if (server->getPort() == serverConfig.second.port) {
          server->addServerConfig(serverConfig.second);
          break;
        }
      }
    } else {
      LOG_DEBUG("Creating new server config in port:",
                serverConfig.second.port);
      try {
        // Try to create a new server and add it to the list
        servers.emplace_back(std::make_shared<Server>(serverConfig.second));
      } catch (const std::exception& e) {
        LOG_ERROR("Failed to create server on port:", serverConfig.second.port,
                  e.what());
        // Remove the server that failed to initialize
        if (!servers.empty() &&
            servers.back()->getPort() == serverConfig.second.port) {
          servers.pop_back();
        }
      }
    }
  }
}

void ServerManager::handleNoEvents(PollManager& pollManager) {
  LOG_INFO("No events, checking for idle clients or expired cookies");
  checkChildProcesses(pollManager);
  for (auto& server : servers) {
    server->checkIdleClients(pollManager);
    server->getSession().checkExpiredCookies();
  }
  checkForNewChildProcesses(pollManager);
}

void ServerManager::initializePollManager(PollManager& pollManager) {
  for (auto& server : servers) {
    if (server->getSocketFd() < 0) {
      LOG_ERROR("Invalid socket fd for server", server->getServerName());
      continue;
    }
    pollManager.addFd(server->getSocketFd(), EPOLLIN,
                      [&](int fd) { (void)fd; });
    LOG_DEBUG("Added server", server->getServerName(), "to pollManager");
  }
}

void ServerManager::startupMessage(void) {
  for (auto& server : servers) {
    LOG_INFO("Server", server->getServerName(), "listening on port",
             server->getPort());
  }
}

void ServerManager::runServers(void) {
  PollManager pollManager;
  LOG_TRACE("Adding server sockets to pollManager");
  initializePollManager(pollManager);
  startupMessage();
  while (!Utility::signalReceived()) {
    int epollCount = pollManager.epollWait();
    LOG_TRACE("Epoll count:", epollCount);
    if (epollCount == -1) {
      if (!Utility::signalReceived()) {
        LOG_ERROR("Failed to epoll fds");
      }
      continue;
    } else if (epollCount == 0) {
      RuntimeException::tryCatch(&ServerManager::handleNoEvents, this,
                                 pollManager);
    } else {
      RuntimeException::tryCatch(&ServerManager::serverLoop, this, pollManager);
    }
  }
}

void ServerManager::serverLoop(PollManager& pollManager) {
  checkChildProcesses(pollManager);
  for (auto& event : pollManager.getEpollEvents()) {
    if (event.data.fd == -1) {
      continue;
    }
    if (pollManager.isValidFd(event.data.fd) == false) {
      continue;
    }
    if (event.events & EPOLLERR || event.events & EPOLLHUP ||
        event.events & EPOLLRDHUP) {
      handlePollErrors(pollManager, event);
    } else if (event.events & EPOLLIN) {
      handlePollInEvent(pollManager, event);
    } else if (event.events & EPOLLOUT) {
      handlePollOutEvent(pollManager, event);
    }
    LOG_TRACE("Handled event for fd:", (int)event.data.fd);
  }
  LOG_TRACE("Checking for new child processes");
  checkForNewChildProcesses(pollManager);
  LOG_TRACE("Finished handling events");
}

void ServerManager::handlePollErrors(PollManager& pollManager,
                                     struct epoll_event& event) {
  int fd = event.data.fd;
  int clientFd = Utility::getClientFdFromCgiParams(fd);
/*   if (!pollManager.isValidFd(clientFd)) {
    LOG_DEBUG("Invalid fd:", clientFd, "cgi fd:", fd);
    pollManager.removeFd(fd);
    return;
  } */
  if ((event.events & EPOLLHUP || event.events & EPOLLERR) &&
      Utility::isCgiFd(fd)) {
    LOG_DEBUG("EPoll hangup on CGI fd:", fd);
    Utility::setIsExited(fd, true);
    for (auto& server : servers) {
      if (server->isClientFd(clientFd)) {
        if (Utility::isOutReadFd(fd)) {
          server->handleClientIn(pollManager, EPOLLIN, fd, clientFd);
        } else {
          server->handleClientIn(pollManager, EPOLLIN,
                                 Utility::getOutReadFdFromClientFd(clientFd),
                                 clientFd);
        }
        break;
      }
    }
    pollManager.removeFd(Utility::getOutReadFdFromClientFd(clientFd));
    pollManager.removeFd(Utility::getInWriteFdFromClientFd(clientFd));
  } else if (event.events & EPOLLERR) {
    LOG_ERROR("Epoll error on fd:", fd);
    pollManager.removeFd(fd);
  } else if (event.events & EPOLLRDHUP) {
    LOG_INFO("Epoll hangup on fd:", fd);
    pollManager.removeFd(fd);
  } else {
    LOG_WARN("EPoll on fd:", fd);
    LOG_WARN("errno:", IException::expandErrno());
    pollManager.removeFd(fd);
  }
}

void ServerManager::handlePollInEvent(PollManager& pollManager,
                                      struct epoll_event& event) {
  LOG_DEBUG("Handling POLLIN event");
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
    } else if (Utility::isCgiFd(fd) &&
               server->isClientFd(Utility::getClientFdFromCgiParams(fd))) {
      int clientFd = Utility::getClientFdFromCgiParams(fd);
      LOG_DEBUG("Handling CGI", fd,
                "EPOLLIN communication with clientFd:", clientFd);
      server->handleClientIn(pollManager, event.events, fd, clientFd);
      break;
    }
  }
  if (!pollManager.isValidFd(fd)) {
    LOG_TRACE("got invalid fd:", fd);
  }
}

void ServerManager::handlePollOutEvent(PollManager& pollManager,
                                       struct epoll_event& event) {
  LOG_DEBUG("Handling POLLOUT event");
  int fd = event.data.fd;
  for (auto& server : servers) {
    if (server->isClientFd(fd)) {
      LOG_TRACE("Client socket ready for writing");
      LOG_DEBUG("Handling client", fd, "POLLOUT communication");
      server->handleClientOut(pollManager, event.events, fd, fd);
      break;
    }
    if (Utility::isCgiFd(fd) &&
        server->isClientFd(Utility::getClientFdFromCgiParams(fd))) {
      int clientFd = Utility::getClientFdFromCgiParams(fd);
      LOG_DEBUG("Handling CGI", fd,
                " POLLOUT communication with clientFd:", clientFd);
      server->handleClientOut(pollManager, event.events, fd, clientFd);
      break;
    }
  }
  if (!pollManager.isValidFd(fd)) {
    LOG_TRACE("got invalid fd:", fd);
  } else {
    LOG_TRACE("got valid fd:", fd);
  }
}

void ServerManager::checkForNewChildProcesses(PollManager& pollManager) {
  for (auto& cgiParam : g_CgiParams) {
    if (!pollManager.fdExists(cgiParam.outReadFd)) {
      LOG_DEBUG("Adding cgi out read fd:", cgiParam.outReadFd,
                " to pollManager");
      Utility::setNonBlocking(cgiParam.outReadFd);
      pollManager.addFd(cgiParam.outReadFd, EPOLLIN,
                        [&](int fd) { close(fd); });
    }
    if (!pollManager.fdExists(cgiParam.inWriteFd)) {
      LOG_DEBUG("Adding cgi in write fd:", cgiParam.inWriteFd,
                " to pollManager");
      Utility::setNonBlocking(cgiParam.inWriteFd);
      pollManager.addFd(cgiParam.inWriteFd, EPOLLOUT,
                        [&](int fd) { close(fd); });
    }
  }
}

bool ServerManager::childTimeout(steady_time_point_t& start) {
  auto now = std::chrono::steady_clock::now();
  if (now - start > std::chrono::seconds(CHILD_TIMEOUT)) {
    LOG_DEBUG("Child process timed out");
    return true;
  }
  return false;
}

void ServerManager::checkChildProcesses(PollManager& pollManager) {
  for (auto it = g_CgiParams.begin(); it != g_CgiParams.end();) {
    LOG_TRACE("Checking child process:", it->pid);
    if (!it->isExited) {
      pid_t result = waitpid(it->pid, &it->childExitStatus, WNOHANG);
      if (result == -1) {
        LOG_ERROR("Failed to wait for child process:",
                  IException::expandErrno());
        pollManager.removeFd(it->outReadFd);
        pollManager.removeFd(it->inWriteFd);
        g_CgiParams.erase(it);
      } else if (result > 0) {  // Child process has exited
        it->isExited = true;
        //close(it->outWriteFd);
        //close(it->inReadFd);
        LOG_TRACE("client fd:", it->clientFd);
        // pollManager.removeFd(it->outReadFd);
        // pollManager.removeFd(it->inWriteFd);
        LOG_DEBUG("Child process", it->pid,
                  "exited with status:", it->childExitStatus);
        if (it->childExitStatus != 0) {
          LOG_DEBUG("Child process exited with status:", it->childExitStatus);
          it->isFailed = true;
        }
      } else if (result == 0 && childTimeout(it->start)) {
        LOG_DEBUG("Child process", it->pid, "timed out");
        kill(it->pid, SIGKILL);
        it->isTimeout = true;
      }
    }
    ++it;
  }
}

// void ServerManager::checkChildProcesses(PollManager& pollManager) {
//   std::vector<int> fdsToRemove;

//   for (auto it = g_CgiParams.begin(); it != g_CgiParams.end();) {
//     if (it->childExitStatus == -1) {
//       pid_t result = waitpid(it->pid, &it->childExitStatus, WNOHANG);
//       if (result == -1) {  // Error waiting for child process
//         LOG_ERROR("Failed to wait for child process:", IException::expandErrno());
//       } else if (result > 0) {  // Child process has exited
//         it->isExited = true;
//         if (it->childExitStatus != 0) {
//           LOG_DEBUG("Child process exited with status:", it->childExitStatus);
//           it->isFailed = true;
//         }
//       } else if (result == 0 && childTimeout(it->start)) {  // Timeout case
//         LOG_DEBUG("Child process", it->pid, "timed out");
//         kill(it->pid, SIGKILL);
//         it->isTimeout = true;
//       }

//       // If the child process finished or had an error, mark FDs for removal
//       if (result != 0 || result == -1) {  // Exited, timed out, or wait error
//         fdsToRemove.push_back(it->outReadFd);
//         fdsToRemove.push_back(it->inWriteFd);
//         it = g_CgiParams.erase(it);  // Erase and update iterator
//         continue;  // Skip iterator increment
//       }
//     }
//     ++it;
//   }

//   // Remove FDs from PollManager after iterating, to avoid segfaults
//   for (int fd : fdsToRemove) {
//     pollManager.removeFd(fd);
//   }
// }