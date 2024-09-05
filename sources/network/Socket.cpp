#include <Socket.hpp>

Socket::Socket(void) {
  LOG_DEBUG(Utility::getConstructor(*this));
}

// Socket::Socket(ServerConfig& serverConfig) {
//   LOG_DEBUG(Utility::getConstructor(*this));
// }

Socket::~Socket(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
  cleanupSocket();
}

void Socket::setupSocket(ServerConfig& serverConfig) {
  struct addrinfo hints = {}, *res;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (int s = getaddrinfo(NULL, std::to_string(serverConfig.port).c_str(), &hints, &res) != 0) {
    socketError("Failed to get address info:", gai_strerror(s));
  }
  sockFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockFd == -1) {
    socketError("Failed to create socket:");
  }
  setNonBlocking();
  int opt = 1;
  // replaced SO_REUSEADDR with SO_REUSEPORT to allow multiple servers to bind to the same port
  if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
    socketError("Failed to set socket options:");
  }
  LOG_DEBUG("Socket fd:", sockFd);
  if (bind(sockFd, res->ai_addr, res->ai_addrlen) == -1) {
    socketError("Failed to bind socket fd:", sockFd);
  }
  if (listen(sockFd, BACKLOG) == -1) {
    socketError("Failed to listen on socket fd:", sockFd);
  }
  freeaddrinfo(res);
}

void Socket::setNonBlocking(void) {
  int flags = fcntl(sockFd, F_GETFL, 0);
  if (flags == -1) {
    socketError("Failed to get socket flags:");
  }
  if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) == -1) {
    socketError("Failed to set socket to non-blocking:");
  }
}

void Socket::cleanupSocket(void) {
  if (sockFd > 0) {
    LOG_DEBUG("cleanupSocket() closing fd:", sockFd);
    if (close(sockFd) == -1) {
      socketError("Failed to close fd:", sockFd);
    }
    sockFd = 0;  // Mark as closed
  }
}
