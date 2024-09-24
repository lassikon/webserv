#include <Socket.hpp>

Socket::Socket(void) : sockFd(0) {
  LOG_TRACE(Utility::getConstructor(*this));
}

Socket::~Socket(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  cleanupSocket();
}

void Socket::setupSocket(ServerConfig& serverConfig) {
  struct addrinfo hints = {}, *addr;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (int s = getaddrinfo(NULL, std::to_string(serverConfig.port).c_str(), &hints, &addr) != 0) {
    throw socketError("Failed to get address info:", gai_strerror(s));
  }
  sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFd == -1) {
    throw socketError("Failed to create socket");
  }
  //setNonBlocking();
  Utility::setNonBlocking(sockFd);
  Utility::setCloseOnExec(sockFd);
  int opt = 1;
  if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    throw socketError("Failed to set socket options");
  }
  LOG_DEBUG("Socket fd:", sockFd);
  if (bind(sockFd, addr->ai_addr, addr->ai_addrlen) == -1) {
    throw socketError("Failed to bind socket fd:", sockFd);
  }
  if (listen(sockFd, BACKLOG) == -1) {
    throw socketError("Failed to listen on socket fd:", sockFd);
  }
  freeaddrinfo(addr);
}

void Socket::setNonBlocking(void) {
  int flags = fcntl(sockFd, F_GETFL, 0);
  if (flags == -1) {
    throw socketError("Failed to get socket flags");
  }
  if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) == -1) {
    throw socketError("Failed to set socket to non-blocking");
  }
}

void Socket::cleanupSocket(void) {
  if (sockFd > 0) {
    LOG_DEBUG("Closing fd:", sockFd);
    if (close(sockFd) == -1) {
      throw socketError("Failed to close fd:", sockFd);
    }
    closeSockedFd();
  }
}
