#include <Client.hpp>

Client::Client(int socketFd) : fd(socketFd) {
  LOG_DEBUG("Client constructor called");
  state = ClientState::READING_REQLINE;
  /*   req.setConfig(config);
    res.setConfig(config); */
}

Client::~Client(void) {
  LOG_DEBUG("Client destructor called");
  cleanupClient();
}

bool Client::operator==(const Client& other) const { return fd == other.fd; }

bool Client::handlePollEvents(short revents) {
  if (revents & POLLIN) {
    if (!receiveData()) {
      return false;
    }
  }
  if (revents & POLLOUT) {
    if (!sendResponse()) {
      return false;
    }
  }
  return true;
}

bool Client::receiveData(void) {
  char buf[4096] = {0};
  int nbytes = recv(fd, buf, sizeof(buf), 0);
  if (nbytes == -1) {  //&& errno != EWOULDBLOCK && errno != EAGAIN
    LOG_ERROR("Failed to recv() from fd:", fd);
    // throw exception
  } else if (nbytes == 0) {  // Connection closed
    LOG_DEBUG("Connection closed for client fd:", fd);
    return false;
  } else {
    LOG_INFO("Receiving data from client fd", fd, ", buffer:", buf);
    // Echo data back to the client
    std::istringstream bufStr(buf);
    processRequest(bufStr);
  }
  return true;
}

// bool Client::receiveData(void) {
//   char buf[4096] = {0};
//   int nbytes = recv(fd, buf, sizeof(buf), 0);
//   if (nbytes == -1 && errno != EWOULDBLOCK && errno != EAGAIN) {
//     LOG_ERROR("Failed to recv() from fd:", fd);
//     // throw exception
//     return false;
//   } else if (nbytes == 0) {  // Connection closed
//     LOG_DEBUG("Connection closed for client fd:", fd);
//     return false;
//   } else {
//     LOG_DEBUG("Receiving data from client fd", fd, ", buffer:", buf);
//     // Echo data back to the client
//     std::string bufStr(buf);
//     processRequest(bufStr);
//     /*     std::string content = "<html><body><h1>Hello,
//        World!</h1></body></html>";

//         std::ostringstream oss;
//         oss << "HTTP/1.1 200 OK\r\n";
//         oss << "Cache-Control: no-cache, private\r\n";
//         oss << "Content-Type: text/html\r\n";
//         oss << "Content-Length: " << content.size() << "\r\n";
//         oss << "\r\n";
//         oss << content;
//         std::string response = oss.str();

//         if (send(fd, response.c_str(), response.size() + 1, 0) == -1) {
//           LOG_ERROR("Send() failed with fd:", fd);
//           // throw exception
//           return false;
//         } */
//   }
//   return true;
// }

void Client::processRequest(std::istringstream& iBuf) {
  LOG_DEBUG("Processing request from client fd:", fd);
  if (state == ClientState::READING_REQLINE) {
    std::string requestLine;
    std::getline(iBuf, requestLine);
    req.parseRequestLine(this, requestLine);
  }
  if (state == ClientState::READING_HEADER) {
    req.parseHeaders(this, iBuf);
  }
  if (state == ClientState::READING_BODY) {
    req.parseBody(this, iBuf);
  }
  if (state == ClientState::READING_DONE) {
    std::cout << "Request received from client fd:" << fd << std::endl;
    handleRequest();
  }
}

void Client::handleRequest(void) {
  LOG_TRACE("Handling request from client fd:", fd);
  if (req.getMethod() == "GET") {
    LOG_INFO("GET request for path:", req.getPath());
    LOG_INFO("Request body:", req.getBody());
    LOG_INFO("Request headers:");
    for (auto& header : req.getHeaders()) {
      LOG_INFO(header.first, ":", header.second);
    }
  } else if (req.getMethod() == "POST") {
    LOG_INFO("POST request for path:", req.getPath());
    LOG_INFO("Request body:", req.getBody());
    LOG_INFO("Request headers:");
    for (auto& header : req.getHeaders()) {
      LOG_INFO(header.first, ":", header.second);
    }
  } else if (req.getMethod() == "DELETE") {
    LOG_INFO("DELETE request for path:", req.getPath());
    LOG_INFO("Request body:", req.getBody());
    LOG_INFO("Request headers:");
    for (auto& header : req.getHeaders()) {
      LOG_INFO(header.first, ":", header.second);
    }
  } else {
    LOG_ERROR("Unsupported method:", req.getMethod());
  }
}

bool Client::sendResponse(void) {
  /*
    std::ifstream file("filename", std::ios::binary);
  std::vector<char> content((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>()); */
  LOG_DEBUG("Sending response to client fd:", fd);
  std::ifstream html("webroot/website0/index.html");
  std::stringstream contentBuf;
  contentBuf << html.rdbuf();
  std::string content = contentBuf.str();

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
    return false;
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
