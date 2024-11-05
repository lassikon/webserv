#include <Server.hpp>

Server::Server(ServerConfig& serverConfig) {
  LOG_TRACE(Utility::getConstructor(*this));
  serverConfigs.emplace_back(std::make_shared<ServerConfig>(serverConfig));
  port = serverConfig.port;
  ipAddress = serverConfig.ipAddress;
  socket.setupSocket(serverConfig);
  serverName = serverConfig.serverName;
}

Server::~Server(void) { LOG_TRACE(Utility::getDeconstructor(*this)); }

void Server::addServerConfig(ServerConfig& serverConfig) {
  serverConfigs.emplace_back(std::make_shared<ServerConfig>(serverConfig));
}

// Accepts a new incoming client connection on the server socket.
// Configures the new connection as non-blocking and close-on-exec, adds it to
// the poll manager, and tracks its last activity timestamp.
void Server::acceptConnection(PollManager& pollManager) {
  struct sockaddr_storage theirAddr {};
  socklen_t addrSize = sizeof theirAddr;

  // Accept the incoming connection and obtain the new socket file descriptor
  int newFd = accept(socket.getFd(), (struct sockaddr*)&theirAddr, &addrSize);
  auto it = pollManager.getInterestFdsList().find(newFd);
  if (it != pollManager.getInterestFdsList().end()) {
    LOG_WARN("Client fd:", newFd, "already exists in pollManager");
    // close(newFd);
    return;
  }

  if (newFd < 2) {
    LOG_WARN("Failed to accept new connection:", IException::expandErrno());
    return;
  }
  Utility::setCloseOnExec(newFd);
  Utility::setNonBlocking(newFd);
  clients.emplace_back(std::make_shared<Client>(newFd, serverConfigs, session));
  LOG_INFO("New Client fd:", newFd, "accepted");

  // Add the new client's file descriptor to the poll manager with EPOLLIN
  // events
  pollManager.addFd(newFd, EPOLLIN,
                    [&](int fd) { removeClient(pollManager, fd); });
  clientLastActivity[newFd] = std::chrono::steady_clock::now();
  LOG_DEBUG("Added Client fd:", newFd, "to pollManager");
}

// Handles EPOLLIN events for a client connection.
// This function finds the client by its file descriptor, processes the incoming
// event, and modifies the client's poll events if needed. If the client should
// be closed, it removes it from the poll manager.
void Server::handleClientIn(PollManager& pollManager, uint32_t revents,
                            int eventFd, int clientFd) {
  auto it = std::find_if(clients.begin(), clients.end(),
                         [clientFd](std::shared_ptr<Client>& client) {
                           return client->getFd() == clientFd;
                         });
  if (it == clients.end()) {
    LOG_ERROR("Client fd:", clientFd, "not found in clients");
    return;
  }
  if (it != clients.end()) {
    bool isClose = false;
    // Check if the event is for CGI or the main client socket
    if (eventFd == Utility::getOutReadFdFromClientFd(clientFd)) {
      LOG_DEBUG("Handling EPOLLIN event for cgi,", eventFd,
                " to client fd:", clientFd);
      isClose = (*it)->handleEpollEvents(revents, eventFd, clientFd);
      updateClientLastActivity(clientFd);
    } else if (eventFd == clientFd) {
      LOG_DEBUG("Handling EPOLLIN event for fd:,", eventFd,
                " to client fd:", clientFd);
      isClose = (*it)->handleEpollEvents(revents, eventFd, clientFd);
      updateClientLastActivity(clientFd);
    }

    // Modify the client's poll events based on its current state

    if (isClose == true) {
      LOG_TRACE("Closing client fd:", clientFd);
      LOG_DEBUG("Removing client fd:", clientFd, "from pollManager");
      removeClient(pollManager, clientFd);
      // pollManager.removeFd(clientFd);
      return;
    }
    modifyFdEvent(pollManager, *it, eventFd, clientFd);
  }
}

// Handles EPOLLOUT events for a client connection.
// This function locates the client by its file descriptor, processes the
// outgoing event, modifies the client's poll events if needed, and removes the
// client from the poll manager if it should be closed.
void Server::handleClientOut(PollManager& pollManager, uint32_t revents,
                             int eventFd, int clientFd) {
  auto it = std::find_if(clients.begin(), clients.end(),
                         [clientFd](std::shared_ptr<Client>& client) {
                           return client->getFd() == clientFd;
                         });
  if (it == clients.end()) {
    LOG_ERROR("Client fd:", clientFd, "not found in clients");
    return;
  }
  if (it != clients.end()) {
    bool isClose = false;
    // Check if the event is for CGI or the main client socket
    if (eventFd == Utility::getInWriteFdFromClientFd(clientFd)) {
      LOG_DEBUG("Handling EPOLLOUT event for cgi,", eventFd,
                " to client fd:", clientFd);
      isClose = (*it)->handleEpollEvents(revents, clientFd, eventFd);
      updateClientLastActivity(clientFd);
    } else if (eventFd == clientFd) {
      LOG_DEBUG("Handling EPOLLOUT event for fd:,", eventFd,
                " to client fd:", clientFd);
      isClose = (*it)->handleEpollEvents(revents, eventFd, clientFd);
      updateClientLastActivity(clientFd);
    }

    // Modify the client's poll events based on its current state

    if (isClose == true) {
      LOG_DEBUG("Closing client fd:", clientFd);
      LOG_DEBUG("Removing client fd:", clientFd, "from pollManager");
      removeClient(pollManager, clientFd);
      // pollManager.removeFd(clientFd);
      return;
    }
    modifyFdEvent(pollManager, *it, eventFd, clientFd);
  }
}

void Server::modifyFdEvent(PollManager& pollManager,
                           std::shared_ptr<Client> client, int eventFd,
                           int clientFd) {
  uint32_t newEvents = 0;
  int fd = clientFd;
  LOG_TRACE("Modifying events for client fd:", clientFd);
  LOG_TRACE("Client state:", (int)client->getClientState());
  LOG_TRACE("CGI state:", (int)client->getCgiState());
  LOG_TRACE("Parsing state:", (int)client->getParsingState());
  // closing connection, detected close in client side
  // removing fd in epoll interest list fds
  if (eventFd == clientFd && client->getClientState() == ClientState::CLOSE &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_DEBUG("EOF/Client closed writing end, CLOSING client fd:", clientFd);
    fd = eventFd;
    LOG_DEBUG("Removing client fd:", clientFd, "from pollManager");
    pollManager.removeFd(clientFd);
    return;
  }
  // client reset to waiting for EPOLLIN / initial state
  if (eventFd == clientFd && client->getClientState() == ClientState::IDLE &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_DEBUG("Client:", clientFd, "Idle state, EPOLLIN, ~EPOLLOUT");
    newEvents &= ~EPOLLOUT;
    newEvents |= EPOLLIN;

    fd = eventFd;
  }
  // after epollin client reading request, waiting for eof in request reading
  if (eventFd == clientFd && client->getClientState() == ClientState::READING &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_DEBUG("Client:", clientFd, "Reading state, EPOLLIN, ~EPOLLOUT");
    newEvents &= ~EPOLLOUT;
    newEvents |= EPOLLIN;
    fd = eventFd;
  }
  // after reading eof request disabling epollin and enabling epollout for
  // client
  if (eventFd == clientFd &&
      client->getClientState() == ClientState::PROCESSING &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_DEBUG("Client:", clientFd, "Processing state, ~EPOLLIN, EPOLLOUT");
    newEvents &= ~EPOLLIN;
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  // after sending response to client, waiting for eof on send
  if (eventFd == clientFd &&
      client->getClientState() == ClientState::PREPARING &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_TRACE("Client:", clientFd, "Sending state, EPOLLOUT");
    newEvents &= ~EPOLLIN;
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  // after sending response to client, waiting for eof on send
  if (eventFd == clientFd && client->getClientState() == ClientState::SENDING &&
      client->getCgiState() == CgiState::IDLE) {
    LOG_TRACE("Client:", clientFd, "Sending state, EPOLLOUT");
    newEvents &= ~EPOLLIN;
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }
  // disable epollin and epollout for Client after cgi post, sending body to cgi
  // epollout to cgi stdin pipe
  if (eventFd == clientFd &&
      client->getClientState() == ClientState::PROCESSING &&
      client->getCgiState() == CgiState::READING) {
    LOG_DEBUG("Client:", clientFd, "Processing state, ~EPOLLIN, ~EPOLLOUT");
    LOG_DEBUG("CGI:", Utility::getInWriteFdFromClientFd(clientFd),
              "Reading state");
    newEvents &= ~EPOLLIN;
    newEvents &= ~EPOLLOUT;
    fd = eventFd;
  }
  // disable epollout for CGI after sending body to cgi and eof on send
  if (eventFd == Utility::getInWriteFdFromClientFd(clientFd) &&
      client->getClientState() == ClientState::READING &&
      client->getCgiState() == CgiState::WRITING) {
    LOG_DEBUG("Client:", clientFd, "Reading state, ~EPOLLOUT");
    LOG_DEBUG("CGI:", Utility::getOutReadFdFromClientFd(clientFd),
              "Reading done, ~EPOLLOUT");
    newEvents &= ~EPOLLOUT;
    fd = eventFd;
  }

  // Disabling epollin and epollout for client after client pollout for cgi
  // request Epollin from cgi stdout
  if (eventFd == clientFd && client->getClientState() == ClientState::READING &&
      client->getCgiState() == CgiState::WRITING) {
    LOG_DEBUG("Client:", clientFd, "Reading state");
    LOG_DEBUG("CGI writing state");
    newEvents &= ~EPOLLIN;
    newEvents &= ~EPOLLOUT;
    fd = eventFd;
  }
  // cgi error before fork
  if (eventFd == clientFd && client->getClientState() == ClientState::READING &&
      client->getCgiState() == CgiState::WRITING &&
      Utility::getOutReadFdFromClientFd(clientFd) == -1) {
    LOG_DEBUG("Client:", clientFd, "Reading state");
    LOG_DEBUG("CGI writing state");
    newEvents |= EPOLLOUT;
    fd = eventFd;
  }

  // CGI events after cgi epollin, waiting for cgi/ client reading eof
  if (eventFd == Utility::getOutReadFdFromClientFd(clientFd) &&
      client->getClientState() == ClientState::READING &&
      client->getCgiState() == CgiState::WRITING) {
    LOG_DEBUG("Client:", clientFd, "Reading state");
    LOG_DEBUG("CGI:", eventFd, "writing state, EPOLLIN, ~EPOLLOUT");
    newEvents = EPOLLIN;
    fd = eventFd;
  }
  // enabling client epollout to send cgi output to client browser
  if (eventFd == Utility::getOutReadFdFromClientFd(clientFd) &&
      client->getClientState() == ClientState::PROCESSING &&
      client->getCgiState() == CgiState::DONE) {
    LOG_DEBUG("Client:", clientFd, "Processing state , EPOLLOUT");
    LOG_DEBUG("CGI:", eventFd, "done state");
    newEvents |= EPOLLOUT;
    fd = Utility::getClientFdFromCgiParams(eventFd);
  }

  if (eventFd == Utility::getOutReadFdFromClientFd(clientFd) &&
      client->getClientState() == ClientState::PREPARING &&
      client->getCgiState() == CgiState::DONE) {
    LOG_DEBUG("Client:", clientFd, "Processing state , EPOLLOUT");
    LOG_DEBUG("CGI:", eventFd, "done state");
    newEvents |= EPOLLOUT;
    pollManager.removeFd(eventFd);
    fd = Utility::getClientFdFromCgiParams(eventFd);
  }

  // Client events after cgi done state, waiitng for sending eof
  if (eventFd == clientFd && client->getClientState() == ClientState::SENDING &&
      client->getCgiState() == CgiState::DONE) {
    LOG_DEBUG("Client:", clientFd, "Sending state, EPOLLOUT");
    LOG_DEBUG("CGI:", Utility::getOutReadFdFromClientFd(clientFd),
              "done state");
    newEvents &= ~EPOLLIN;
    newEvents |= EPOLLOUT;
    fd = eventFd;
    pollManager.removeFd(Utility::getOutReadFdFromClientFd(clientFd));
    pollManager.removeFd(Utility::getInWriteFdFromClientFd(clientFd));
    client->getCgiHandler().closePipeFds();
    // pollManager.removeFd(eventFd);
  }
  pollManager.modifyFd(fd, newEvents);
}

void Server::checkIdleClients(PollManager& pollManager) {
  auto now = std::chrono::steady_clock::now();
  for (auto it = clientLastActivity.begin(); it != clientLastActivity.end();
       it++) {
    if (now - it->second > idleTimeout) {
      LOG_DEBUG("Client fd:", it->first, "has been idle for",
                idleTimeout.count(), "seconds");
      LOG_DEBUG("Removing client fd:", it->first, "from pollManager");
      pollManager.removeFd(it->first);
      break;
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

void Server::removeClient(PollManager& pollManager, int clientFd) {
  LOG_DEBUG("Removing client fd:", clientFd, "from clients");
  auto it = std::find_if(clients.begin(), clients.end(),
                         [clientFd](std::shared_ptr<Client>& client) {
                           return client->getFd() == clientFd;
                         });
  if (it != clients.end()) {
    clients.erase(it);
  }
  for (auto cgi = g_CgiParams.begin(); cgi != g_CgiParams.end();) {
    if (cgi->clientFd == clientFd) {
      pollManager.removeFd(cgi->outReadFd);
      pollManager.removeFd(cgi->inWriteFd);
      kill(cgi->pid, SIGKILL);
      LOG_TRACE("Killed CGI process:", cgi->pid);
      break;
    } else {
      ++cgi;
    }
  }

  for (auto it = clientLastActivity.begin(); it != clientLastActivity.end();) {
    if (it->first == clientFd) {
      clientLastActivity.erase(it);
      break;
    } else {
      it++;
    }
  }
}
