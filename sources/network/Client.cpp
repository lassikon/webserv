#include <Client.hpp>

Client::Client(int socketFd) : fd(socketFd) { LOG_DEBUG("Client constructor called"); }

Client::~Client(void) {
  LOG_DEBUG("Client destructor called");
  cleanupClient();
}

bool Client::operator==(const Client& other) const { return fd == other.fd; }

bool Client::receiveData(void) {
  char buf[4096] = {0};
  int nbytes = recv(fd, buf, sizeof(buf), 0);
  if (nbytes == -1 && errno != EWOULDBLOCK && errno != EAGAIN) {
    LOG_ERROR("Failed to recv() from fd:", fd);
    // throw exception
    return false;
  } else if (nbytes == 0) {  // Connection closed
    LOG_DEBUG("Connection closed for client fd:", fd);
    return false;
  } else {
    LOG_DEBUG("Receiving data from client fd", fd, ", buffer:", buf);
    // Echo data back to the client
    if (send(fd, buf, nbytes, 0) == -1) {
      LOG_ERROR("Send() failed with fd:", fd);
      // throw exception
      return false;
    }
  }
  return true;
}

void Client::setFd(int newFd) {
  if (fd != newFd) {
    LOG_DEBUG("Changing fd from:", fd, "to:", newFd);
    cleanupClient();
    fd = newFd;
  }
}

void Client::cleanupClient(void) {
  if (fd > 0) {
    LOG_DEBUG("cleanupClient() closing fd:", fd);
    if (close(fd) == -1) {
      LOG_ERROR("Failed to close fd:", fd);
      // throw exception
    }
    fd = -1;  // Mark as closed
  }
}