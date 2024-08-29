#include <Server.hpp>

Server::Server(std::string port)
    : port(port){
  LOG_DEBUG("Server constructor called");
  socket = Socket();
  // clients.reserve(MAX_CLIENTS);
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
    clients.emplace_back(std::make_shared<Client>(newFd));
    LOG_DEBUG("Accepted new client fd:", newFd);
    pollManager.addFd(newFd, POLLIN);
  }
}

void Server::handleClient(PollManager& pollManager, int clientFd, short revents) {
  auto it = std::find_if(
      clients.begin(), clients.end(),
      [clientFd](std::shared_ptr<Client>& client) { return client->getFd() == clientFd; });
  if (it == clients.end()) {
    return;
  }
  if (!(*it)->handlePollEvents(revents)) {  // connection closed or error
    LOG_DEBUG("handleCLient removing client fd:", clientFd);
    pollManager.removeFd(clientFd);
    LOG_DEBUG("handleClient erasing client fd:", (*it)->getFd());
    clients.erase(it);
  }
}


bool Server::isClientFd(int fd) const {
  for (auto& client : clients) {
    if ((*client).getFd() == fd) {
      return true;
    }
  }
  return false;
}