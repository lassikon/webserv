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
  if (newFd == -1) {
    LOG_WARN("Failed to accept new connection:", STRERROR);
    return;
  }
  clients.emplace_back(std::make_shared<Client>(newFd, serverConfigs));
  LOG_DEBUG("Accepted new client fd:", newFd);
  pollManager.addFd(newFd, POLLIN);
  clientLastActivity[newFd] = std::chrono::steady_clock::now();
  LOG_DEBUG("Added client fd:", newFd, "to pollManager");
}

void Server::handleClient(PollManager& pollManager, short revents, int readFd, int clientFd) {
  auto it = std::find_if(
    clients.begin(), clients.end(),
    [clientFd](std::shared_ptr<Client>& client) { return client->getFd() == clientFd; });
  if (it == clients.end()) {
    LOG_ERROR("Client fd:", clientFd, "not found in clients");
    return;
  }
  if ((*it)->handlePollEvents(revents, readFd, clientFd) == false) {  // connection closed or error
    LOG_DEBUG("Removing client fd:", clientFd, "from pollManager");
    pollManager.removeFd(clientFd);
    clientLastActivity.erase(clientFd);
    LOG_DEBUG("Erasing client fd:", (*it)->getFd(), "from clients");
    clients.erase(it);
  } else {
    updateClientLastActivity(clientFd);
  }
}

void Server::checkIdleClients(PollManager& pollManager) {
  auto now = std::chrono::steady_clock::now();
  for (auto it = clientLastActivity.begin(); it != clientLastActivity.end();) {
    if (now - it->second > idleTimeout) {
      LOG_DEBUG("Client fd:", it->first, "has been idle for", idleTimeout.count(), "seconds");
      pollManager.removeFd(it->first);
      it = clientLastActivity.erase(it);
    } else {
      ++it;
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
