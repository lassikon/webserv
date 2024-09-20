#include <Server.hpp>

Server::Server(ServerConfig& serverConfig) {
  LOG_TRACE(Utility::getConstructor(*this));
  serverConfigs.emplace_back(std::make_shared<ServerConfig>(serverConfig));
  port = serverConfig.port;
  ipAddress = serverConfig.ipAddress;
  socket = Socket();
  socket.setupSocket(serverConfig);
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
  if (newFd == -1) {
    LOG_WARN("Failed to accept new connection:", IException::expandErrno());
    return;
  }
  clients.emplace_back(std::make_shared<Client>(newFd, serverConfigs));
  LOG_DEBUG("Accepted new client fd:", newFd);
  pollManager.addFd(newFd, EPOLLIN);
  clientLastActivity[newFd] = std::chrono::steady_clock::now();
  LOG_DEBUG("Added client fd:", newFd, "to pollManager");
}

void Server::handleClient(PollManager& pollManager, uint32_t revents, int eventFd, int clientFd) {
  auto it = std::find_if(
    clients.begin(), clients.end(),
    [clientFd](std::shared_ptr<Client>& client) { return client->getFd() == clientFd; });

  if (it == clients.end()) {
    LOG_ERROR("Client fd:", clientFd, "not found in clients");
    return;
  }
  if (it != clients.end()) {
    if (eventFd == Utility::getInWriteFdFromClientFd(clientFd)) {
      LOG_DEBUG("Handling POLLOUT event for cgi,", eventFd, " to client fd:", clientFd);
      (*it)->handleEpollEvents(revents, clientFd, eventFd);
      updateClientLastActivity(clientFd);
    } else {
      LOG_DEBUG("Handling POLLIN event for fd:,", eventFd, " to client fd:", clientFd);
      (*it)->handleEpollEvents(revents, eventFd, clientFd);
      updateClientLastActivity(clientFd);
    }
    /*     if ((*it)->handlePollEvents(revents, pollFd, clientFd) == false) {
      LOG_TRACE("Connection closed or error occured");
      LOG_DEBUG("Removing client fd:", clientFd, "from pollManager");
      pollManager.removeFd(clientFd);
      clientLastActivity.erase(clientFd);
      LOG_DEBUG("Erasing client fd:", (*it)->getFd(), "from clients");
      clients.erase(it);
    } else {
      updateClientLastActivity(clientFd);
    } */
  }

  uint32_t newEvents = 0;  // Default to listening for input
  int fd;
  if (eventFd == clientFd && (*it)->getClientState() == ClientState::IDLE &&
      (*it)->getCgiState() == CgiState::IDLE) {
    LOG_WARN("Triggering Edge for fd:", clientFd);
    newEvents &= ~EPOLLOUT;
    newEvents |= EPOLLIN;
    fd = eventFd;
  }
  if (eventFd == clientFd && (*it)->getClientState() == ClientState::READING &&
      (*it)->getCgiState() == CgiState::IDLE) {
    LOG_WARN("Disabling POLLOUT for client fd:", clientFd);
    newEvents &= ~EPOLLOUT;
    newEvents |= EPOLLIN ;
    fd = eventFd;
  }
  if (eventFd == clientFd && (*it)->getClientState() == ClientState::READING &&
      (*it)->getCgiState() ==
        CgiState::WRITING) {  //Client events after client poll out for cgi bin/get request
    LOG_WARN("Disabling EVENTS for client fd:", clientFd);
    newEvents = 0;
    fd = eventFd;
  }
  if (eventFd == clientFd && (*it)->getClientState() == ClientState::PROCESSING &&
      (*it)->getCgiState() == CgiState::IDLE) {
    LOG_TRACE("Enabling POLLOUT for client fd:",
              clientFd);  //Client events after client poll in simple request
    newEvents &= ~EPOLLIN;
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  if (Utility::isCgiFd(eventFd) && (*it)->getClientState() == ClientState::READING &&
      (*it)->getCgiState() == CgiState::WRITING) {
    LOG_TRACE("Enabling POLLIN event in CGI:", eventFd,
              " for client fd:", clientFd);  //CGI events after cgi poll in  for bin/get request
    newEvents = EPOLLIN;
    fd = eventFd;
  }
  if (Utility::isCgiFd(eventFd) && (*it)->getClientState() == ClientState::PROCESSING &&
      (*it)->getCgiState() == CgiState::DONE) {
    LOG_TRACE("Enabling POLLOUT for client fd:", clientFd,
              "after CGI fd:", eventFd);  //CGI events after cgi poll in  for bin/get request
    newEvents |= EPOLLOUT;
    fd = Utility::getClientFdFromCgiParams(eventFd);
  }
  if (eventFd == clientFd && (*it)->getClientState() == ClientState::SENDING) {
    LOG_TRACE("Enabling POLLOUT for client fd:", clientFd);
    newEvents &= ~EPOLLIN;
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  if (eventFd == clientFd && (*it)->getClientState() == ClientState::SENDING &&
      (*it)->getCgiState() == CgiState::DONE) {
    LOG_TRACE("Enabling POLLOUT for client fd:", clientFd);
    newEvents &= ~EPOLLIN;
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  if (eventFd == Utility::getInWriteFdFromClientFd(clientFd) &&
      (*it)->getCgiState() == CgiState::WRITING) {
    LOG_TRACE("disabling POLLOUT for cgi fd:", eventFd);
    newEvents &= ~EPOLLOUT;
    fd = eventFd;
  }
  if (eventFd == clientFd && (*it)->getCgiState() == CgiState::DONE &&
      (*it)->getClientState() == ClientState::PROCESSING) {
    LOG_TRACE("Enabling POLLOUT for client fd:", clientFd);
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  if (eventFd == Utility::getInWriteFdFromClientFd(clientFd) &&
      (*it)->getCgiState() == CgiState::READING) {
    LOG_TRACE("Enabling POLLOUT for cgi fd:", eventFd);
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  pollManager.modifyFd(fd, newEvents);
}

void Server::checkIdleClients(PollManager& pollManager) {
  auto now = std::chrono::steady_clock::now();
  for (auto it = clientLastActivity.begin(); it != clientLastActivity.end();) {
    if (now - it->second > idleTimeout) {
      LOG_DEBUG("Client fd:", it->first, "has been idle for", idleTimeout.count(), "seconds");
      pollManager.removeFd(it->first);
      it = clientLastActivity.erase(it);
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
