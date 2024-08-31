#include <Socket.hpp>

Socket::Socket() : sockFd(0) { LOG_DEBUG("Socket constructor called"); }

Socket::~Socket(void) {
  LOG_DEBUG("Socket destructor called");
  cleanupSocket();
}

void Socket::setupSocket(int port) {
  struct addrinfo hints, *res;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &res) != 0) {
    LOG_ERROR("Failed to get address info");
    // gai_strerror()
    // throw exception
  }
  sockFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockFd == -1) {
    LOG_ERROR("Failed to create socket");
    // throw exception
  }
  setNonBlocking();
  int opt = 1;
  if (setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    LOG_ERROR("Failed to set socket options");
    cleanupSocket();
    // throw exception
  }
  LOG_DEBUG("Socket fd:", sockFd);
  if (bind(sockFd, res->ai_addr, res->ai_addrlen) == -1) {
    LOG_ERROR("Failed to bind socket fd:", sockFd);
    cleanupSocket();
    // throw exception
  }
  if (listen(sockFd, BACKLOG) == -1) {
    LOG_ERROR("Failed to listen on socket fd:", sockFd);
    cleanupSocket();
    // throw exception
  }
  freeaddrinfo(res);
}

void Socket::setNonBlocking(void) {
  int flags = fcntl(sockFd, F_GETFL, 0);
  if (flags == -1) {
    LOG_ERROR("Failed to get socket flags");
    cleanupSocket();
    // throw exception
  }
  if (fcntl(sockFd, F_SETFL, flags | O_NONBLOCK) == -1) {
    LOG_ERROR("Failed to set socket to non-blocking");
    cleanupSocket();
    // throw exception
  }
}

void Socket::cleanupSocket(void) {
  if (sockFd > 0) {
    LOG_DEBUG("cleanupSocket() closing fd:", sockFd);
    if (close(sockFd) == -1) {
      LOG_ERROR("Failed to close fd:", sockFd);
      // throw exception
    }
    sockFd = 0;  // Mark as closed
  }
}