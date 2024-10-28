#include <Socket.hpp>

Socket::Socket(void) : sockFd(0) {
  LOG_TRACE(Utility::getConstructor(*this));
}

Socket::~Socket(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  cleanupSocket();
}

// Configures the socket with appropriate options, binds it to the specified port,
// and begins listening for incoming connections.
void Socket::setupSocket(ServerConfig& serverConfig) {
  struct addrinfo hints = {}, *addr;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  // Get address information for binding the socket
  int s = getaddrinfo(nullptr, std::to_string(serverConfig.port).c_str(), &hints, &addr);
  if (s != 0) {
    throw socketError("Failed to get address info:", gai_strerror(s));
  }

  // Create a new socket file descriptor
  sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFd == -1) {
    throw socketError("Failed to create socket");
  }
  Utility::setNonBlocking(sockFd);
  Utility::setCloseOnExec(sockFd);

  // Enable address reuse to avoid "Address already in use" errors on restart
  int opt = 1;
  if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    throw socketError("Failed to set socket options");
  }
  LOG_DEBUG("Socket fd:", sockFd);

  // Bind the socket to the address and port specified by addr
  if (bind(sockFd, addr->ai_addr, addr->ai_addrlen) == -1) {
    throw socketError("Failed to bind socket fd:", sockFd);
  }

  // Begin listening for incoming connections with a defined backlog size
  if (listen(sockFd, BACKLOG) == -1) {
    throw socketError("Failed to listen on socket fd:", sockFd);
  }

  freeaddrinfo(addr);
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
