#include <Client.hpp>

Client::Client(int socket_fd) : fd(socket_fd) {}

Client::~Client(void) { close(fd); }

bool Client::operator==(const Client& other) { return fd == other.fd; }

bool Client::receiveData(void) {
  char buf[1024];
  int nbytes = recv(fd, buf, sizeof(buf), 0);
  if (nbytes <= 0) {
    if (nbytes == 0 || (errno != EWOULDBLOCK && errno != EAGAIN)) {
      return false;  // Connection closed or error
    }
  } else {
    LOG_INFO("receiving data from client fd ", fd, ", buffer: ", buf);
    // Echo data back to the client
    send(fd, buf, nbytes, 0);
  }
  return true;
}

int Client::getFd(void) { return fd; }

void Client::setFd(int fd) { this->fd = fd; }