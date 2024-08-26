#include <Client.hpp>

Client::Client(int socketFd) : fd(socketFd) {}

Client::~Client(void) {
  if (fd > 0) {
    if (close(fd) == -1) {
      LOG_ERROR("failed to close fd:", fd);
      // throw exception
    }
  }
}

bool Client::operator==(const Client& other) { return fd == other.fd; }

bool Client::receiveData(void) {
  char buf[4096];
  memset(buf, 0, sizeof(buf));
  int nbytes = recv(fd, buf, sizeof(buf), 0);
  if (nbytes <= 0) {
    if (nbytes == 0) {  //|| (errno != EWOULDBLOCK && errno != EAGAIN)) {
      return false;     // Connection closed or error
    }
  } else {
    LOG_INFO("receiving data from client fd", fd, ", buffer:", buf);
    // Echo data back to the client
    if (send(fd, buf, nbytes, 0) == -1) {
      LOG_ERROR("send() failed with fd:", fd);
      // throw exception
    }
  }
  return true;
}

int Client::getFd(void) { return fd; }

void Client::setFd(int fd) { this->fd = fd; }