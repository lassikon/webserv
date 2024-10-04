#include <Server.hpp>

Server::Server(ServerConfig& serverConfig) {
  LOG_TRACE(Utility::getConstructor(*this));
  serverConfigs.emplace_back(std::make_shared<ServerConfig>(serverConfig));
  port = serverConfig.port;
  ipAddress = serverConfig.ipAddress;
  socket = Socket();
  socket.setupSocket(serverConfig);
  // session = SessionManager(*this);
}

Server::~Server(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
}

void Server::addServerConfig(ServerConfig& serverConfig) {
  serverConfigs.emplace_back(std::make_shared<ServerConfig>(serverConfig));
}

void Server::acceptConnection(PollManager& pollManager) {
  struct sockaddr_storage theirAddr {};
  socklen_t addrSize = sizeof theirAddr;
  int newFd = accept(socket.getFd(), (struct sockaddr*)&theirAddr, &addrSize);
  if (newFd == -1 || newFd < 2) {
    LOG_WARN("Failed to accept new connection:", IException::expandErrno());
    return;
  }
  Utility::setCloseOnExec(newFd);
  clients.emplace_back(std::make_shared<Client>(newFd, serverConfigs));
  LOG_DEBUG("Accepted new client fd:", newFd);
  pollManager.addFd(newFd, EPOLLIN);
  clientLastActivity[newFd] = std::chrono::steady_clock::now();
  LOG_DEBUG("Added client fd:", newFd, "to pollManager");
}

void Server::handleClientIn(PollManager& pollManager, uint32_t revents, int eventFd, int clientFd) {
  bool isClose = false;
  auto it = std::find_if(
    clients.begin(), clients.end(),
    [clientFd](std::shared_ptr<Client>& client) { return client->getFd() == clientFd; });
  if (it == clients.end()) {
    LOG_ERROR("Client fd:", clientFd, "not found in clients");
    return;
  }
  if (it != clients.end()) {
    if (eventFd == Utility::getOutReadFdFromClientFd(clientFd)) {
      LOG_DEBUG("Handling EPOLLIN event for cgi,", eventFd, " to client fd:", clientFd);
      isClose = (*it)->handleEpollEvents(revents, eventFd, clientFd);
      updateClientLastActivity(clientFd);
    } else if (eventFd == clientFd) {
      LOG_DEBUG("Handling EPOLLIN event for fd:,", eventFd, " to client fd:", clientFd);
      isClose = (*it)->handleEpollEvents(revents, eventFd, clientFd);
      updateClientLastActivity(clientFd);
    }
    if (isClose == true) {
      LOG_TRACE("Connection closed or error occured");
      LOG_DEBUG("Removing client fd:", clientFd, "from pollManager");
      pollManager.removeFd(clientFd);
      clientLastActivity.erase(clientFd);
      LOG_DEBUG("Erasing client fd:", (*it)->getFd(), "from clients");
      clients.erase(it);
    }
  }
  modifyFdEvent(pollManager, *it, eventFd, clientFd);
}

void Server::handleClientOut(PollManager& pollManager, uint32_t revents, int eventFd,
                             int clientFd) {
  bool isClose = false;
  auto it = std::find_if(
    clients.begin(), clients.end(),
    [clientFd](std::shared_ptr<Client>& client) { return client->getFd() == clientFd; });
  if (it == clients.end()) {
    LOG_ERROR("Client fd:", clientFd, "not found in clients");
    return;
  }
  if (it != clients.end()) {
    if (eventFd == Utility::getInWriteFdFromClientFd(clientFd)) {
      LOG_DEBUG("Handling EPOLLOUT event for cgi,", eventFd, " to client fd:", clientFd);
      isClose = (*it)->handleEpollEvents(revents, clientFd, eventFd);
      updateClientLastActivity(clientFd);
    } else if (eventFd == clientFd) {
      LOG_DEBUG("Handling EPOLLOUT event for fd:,", eventFd, " to client fd:", clientFd);
      isClose = (*it)->handleEpollEvents(revents, eventFd, clientFd);
      updateClientLastActivity(clientFd);
    }
    if (isClose == true) {
      LOG_TRACE("Connection closed or error occured");
      pollManager.removeFd(clientFd);
      clientLastActivity.erase(clientFd);
      clients.erase(it);
    }
  }
  modifyFdEvent(pollManager, *it, eventFd, clientFd);
}

void Server::modifyFdEvent(PollManager& pollManager, std::shared_ptr<Client> client, int eventFd,
                           int clientFd) {
  uint32_t newEvents = 0;
  int fd = clientFd;
  //closing connection, detected close in client side
  //removing fd in epoll interest list fds
  if (eventFd == clientFd && client->getClientState() == ClientState::CLOSE &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_INFO("EOF/Client closed writing end, CLOSING client fd:", clientFd);
    fd = eventFd;
    pollManager.removeFd(clientFd);
    for (auto it = clientLastActivity.begin(); it != clientLastActivity.end();) {
      if (it->first == clientFd) {
        clientLastActivity.erase(it);
        break;
      } else {
        it++;
      }
    }
    for (auto it = clients.begin(); it != clients.end();) {
      if ((*it)->getFd() == clientFd) {
        clients.erase(it);
        break;
      } else {
        it++;
      }
    }
    return;
  }
  //client reset to waiting for EPOLLIN / initial state
  if (eventFd == clientFd && client->getClientState() == ClientState::IDLE &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_INFO("Client:", clientFd, "Idle state, EPOLLIN, ~EPOLLOUT");
    newEvents &= ~EPOLLOUT;
    newEvents |= EPOLLIN;
    fd = eventFd;
  }
  // after epollin client reading request, waiting for eof in request reading
  if (eventFd == clientFd && client->getClientState() == ClientState::READING &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_INFO("Client:", clientFd, "Reading state, EPOLLIN, ~EPOLLOUT");
    newEvents &= ~EPOLLOUT;
    newEvents |= EPOLLIN;
    fd = eventFd;
  }
  //after reading eof request disabling epollin and enabling epollout for client
  if (eventFd == clientFd && client->getClientState() == ClientState::PROCESSING &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_INFO("Client:", clientFd, "Processing state, ~EPOLLIN, EPOLLOUT");
    newEvents &= ~EPOLLIN;
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  //after sending response to client, waiting for eof on send
  if (eventFd == clientFd && client->getClientState() == ClientState::SENDING &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_TRACE("Client:", clientFd, "Sending state, EPOLLOUT");
    newEvents &= ~EPOLLIN;
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  //disable epollin and epollout for Client after cgi post, sending body to cgi
  //epollout to cgi stdin pipe
  if (eventFd == clientFd && client->getClientState() == ClientState::PROCESSING &&
      client->getCgiState() == CgiState::READING) {
    LOG_INFO("Client:", clientFd, "Processing state, ~EPOLLIN, ~EPOLLOUT");
    LOG_INFO("CGI:", Utility::getInWriteFdFromClientFd(clientFd), "Reading state");
    newEvents &= ~EPOLLIN;
    newEvents &= ~EPOLLOUT;
    fd = eventFd;
  }
  //disable epollout for CGI after sending body to cgi and eof on send
  if (eventFd == Utility::getInWriteFdFromClientFd(clientFd) &&
      client->getClientState() == ClientState::READING &&
      client->getCgiState() == CgiState::WRITING) {
    LOG_INFO("Client:", clientFd, "Reading state, ~EPOLLOUT");
    LOG_INFO("CGI:", Utility::getOutReadFdFromClientFd(clientFd), "Reading done, ~EPOLLOUT");
    newEvents &= ~EPOLLOUT;
    fd = eventFd;
  }
  /* 
  if (eventFd == clientFd && client->getClientState() == ClientState::DONE &&
      client->getCgiState() ==
        CgiState::WRITING) {  //Client events after cgi poll out for bin/get request
    LOG_INFO("Client:", clientFd, "done state, ~EPOLLIN, ~EPOLLOUT for Cgi:",
             Utility::getInWriteFdFromClientFd(clientFd), "writing state");
    newEvents = 0;
    fd = eventFd;
  } */

  //Disabling epollin and epollout for client after client pollout for cgi request
  //Epollin from cgi stdout
  if (eventFd == clientFd && client->getClientState() == ClientState::READING &&
      client->getCgiState() == CgiState::WRITING) {
    LOG_INFO("Client:", clientFd, "Reading state");
    LOG_INFO("CGI writing state");
    newEvents = 0;
    fd = eventFd;
  }
  //CGI events after cgi epollin, waiting for cgi/reading eof
  if (eventFd == Utility::getOutReadFdFromClientFd(clientFd) &&
      client->getClientState() == ClientState::READING &&
      client->getCgiState() == CgiState::WRITING) {
    LOG_INFO("Client:", clientFd, "Reading state");
    LOG_INFO("CGI:", eventFd, "writing state, EPOLLIN, ~EPOLLOUT");
    newEvents = EPOLLIN;
    fd = eventFd;
  }
  //enabling client epollout to send cgi output to client
  if (eventFd == Utility::getOutReadFdFromClientFd(clientFd) &&
      client->getClientState() == ClientState::PROCESSING &&
      client->getCgiState() == CgiState::DONE) {
    LOG_INFO("Client:", clientFd, "Processing state , EPOLLOUT");
    LOG_INFO("CGI:", eventFd, "done state");
    newEvents |= EPOLLOUT;
    fd = Utility::getClientFdFromCgiParams(eventFd);
  }
  //Client events after cgi done state, waiitng for sending eof
  if (eventFd == clientFd && client->getClientState() == ClientState::SENDING &&
      client->getCgiState() == CgiState::DONE) {
    LOG_INFO("Client:", clientFd, "Sending state, EPOLLOUT");
    LOG_INFO("CGI:", Utility::getOutReadFdFromClientFd(clientFd), "done state");
    newEvents &= ~EPOLLIN;
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  pollManager.modifyFd(fd, newEvents);

  if (eventFd == clientFd && client->getCgiState() == CgiState::DONE) {
    LOG_INFO("Client:", clientFd, "EOF Done state");
    LOG_INFO("CGI:", Utility::getOutReadFdFromClientFd(clientFd), "done state");
    newEvents = 0;
    fd = eventFd;
  }
}

void Server::checkIdleClients(PollManager& pollManager) {
  auto now = std::chrono::steady_clock::now();
  for (auto it = clientLastActivity.begin(); it != clientLastActivity.end();) {
    if (now - it->second > idleTimeout) {
      LOG_DEBUG("Client fd:", it->first, "has been idle for", idleTimeout.count(), "seconds");
      pollManager.removeFd(it->first);
      clientLastActivity.erase(it);
      if (isClientFd(it->first)) {
        auto clientIt = std::find_if(
          clients.begin(), clients.end(),
          [it](std::shared_ptr<Client>& client) { return client->getFd() == it->first; });
        if (clientIt != clients.end()) {
          clients.erase(clientIt);
          break;
        }
      }
    } else {
      it++;
    }
  }
}

void Server::updateClientLastActivity(int clientFd) {
  clientLastActivity[clientFd] = std::chrono::steady_clock::now();
  LOG_DEBUG("Updated last activity for client fd:", clientFd);
}

bool Server::isClientFd(int fd) const {
  for (auto& client : clients) {
    if ((*client).getFd() == fd) {
      return true;
    }
  }
  return false;
}
