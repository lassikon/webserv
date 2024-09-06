#include <Server.hpp>

Server::Server(ServerConfig& serverConfig) {
  LOG_DEBUG(Utility::getConstructor(*this));
  serverConfigs.emplace_back(std::make_shared<ServerConfig>(serverConfig));
  port = serverConfig.port;
  ipAddress = serverConfig.ipAddress;
  socket = Socket();
  socket.setupSocket(serverConfig);
}

Server::~Server(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}

void Server::addServerConfig(ServerConfig& serverConfig) {
  serverConfigs.emplace_back(std::make_shared<ServerConfig>(serverConfig));
}

void Server::acceptConnection(PollManager& pollManager) {
  struct sockaddr_storage theirAddr {};

  socklen_t addrSize = sizeof theirAddr;
  int newFd = accept(socket.getFd(), (struct sockaddr*)&theirAddr, &addrSize);
  // newFd = -1;  // testing error handling, remove this line
  if (newFd == -1) {
    LOG_WARN("Failed to accept new connection:", STRERROR);
    return;
  }
  clients.emplace_back(std::make_shared<Client>(newFd, serverConfigs));
  LOG_DEBUG("Accepted new client fd:", newFd);
  pollManager.addFd(newFd, POLLIN | POLLOUT);
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