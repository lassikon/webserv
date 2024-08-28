#include <Server.hpp>

Server::Server(std::string port) : port(port) {
  LOG_DEBUG("Server constructor called");
  socket = Socket();
  clients.reserve(MAX_CLIENTS);
  socket.setupSocket(port);
}

Server::~Server(void) { LOG_DEBUG("Server destructor called"); }

void Server::acceptConnection(PollManager& pollManager) {
  struct sockaddr_storage theirAddr {};
  socklen_t addrSize = sizeof theirAddr;
  int newFd = accept(socket.getFd(), (struct sockaddr*)&theirAddr, &addrSize);
  if (newFd == -1) {
    LOG_ERROR("Failed to accept client");
    return;
  } else {
    clients.emplace_back(newFd);
    LOG_DEBUG("Accepted new client fd:", newFd);
    pollManager.addFd(newFd, POLLIN);
  }
}

void Server::handleClient(PollManager& pollManager, int clientFd) {
  auto it = std::find_if(clients.begin(), clients.end(),
                         [clientFd](Client& client) { return client.getFd() == clientFd; });
  if (it == clients.end()) {
    return;
  }
  if (!it->receiveData()) {  // connection closed or error
    pollManager.removeFd(clientFd);
    clients.erase(it);
  }
}