#include <Client.hpp>

Client::Client(int socketFd) : fd(socketFd) { LOG_DEBUG("Client constructor called"); }

Client::~Client(void) {
  LOG_DEBUG("Client destructor called");
  if (fd > 0) {
    LOG_DEBUG("Client destructor closing fd:", fd);
    if (close(fd) == -1) {
      LOG_ERROR("Failed to close fd:", fd);
      // throw exception
    }
  }
}

bool Client::operator==(const Client& other) const { return fd == other.fd; }

bool Client::receiveData(void) {
  char buf[4096] = {0};
  int nbytes = recv(fd, buf, sizeof(buf), 0);
  if (nbytes == -1) {
    LOG_ERROR("Failed to recv() from fd:", fd);
    // throw exception
  } else if (nbytes == 0) {  //|| (errno != EWOULDBLOCK && errno != EAGAIN)) {
    return false;            // Connection closed or error
  } else {
    LOG_INFO("Receiving data from client fd", fd, ", buffer:", buf);
    // Echo data back to the client

    std::string content = "<html><body><h1>Hello, World!</h1></body></html>";

    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Cache-Control: no-cache, private\r\n";
    oss << "Content-Type: text/html\r\n";
    oss << "Content-Length: " << content.size() << "\r\n";
    oss << "\r\n";
    oss << content;
    std::string response = oss.str();

    if (send(fd, response.c_str(), response.size() + 1, 0) == -1) {
      LOG_ERROR("Send() failed with fd:", fd);
      // throw exception
    }
  }
  return true;
}

int Client::getFd(void) const { return fd; }

void Client::setFd(int fd) {
  if (this->fd != fd) {
    LOG_DEBUG("setFd closing fd:", fd);
    if (close(this->fd) == -1) {
      LOG_ERROR("Failed to close fd:", this->fd);
      // throw exception
    }
    this->fd = fd;
  }
}