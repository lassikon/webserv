#include <Client.hpp>

Client::Client(int socketFd, std::vector<std::shared_ptr<ServerConfig>>& serverConfigs)
    : fd(socketFd), serverConfigs(serverConfigs) {
  LOG_DEBUG(Utility::getConstructor(*this));
  state = ClientState::READING_REQLINE;
}

Client::~Client(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
  cleanupClient();
}

bool Client::operator==(const Client& other) const {
  return fd == other.fd;
}

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
  if (nbytes == -1) {
    clientError("Failed to recv() from fd:", fd);
  } else if (nbytes == 0) {  // Connection closed
    LOG_TRACE("Connection closed for client fd:", fd);
    return false;
  } else {
    LOG_INFO("Receiving data from client fd", fd, ", buffer:", buf);
    std::istringstream bufStr(buf);
    processRequest(bufStr, nbytes);
  }
  return true;
}

void Client::processRequest(std::istringstream& iBuf, int nbytes) {
  LOG_TRACE("Processing request from client fd:", fd);
  if (state == ClientState::READING_REQLINE) {
    std::string requestLine;
    std::getline(iBuf, requestLine);
    req.parseRequestLine(this, requestLine);
  }
  if (state == ClientState::READING_HEADER) {
    req.parseHeaders(this, iBuf);
  }
  if (state == ClientState::READING_BODY) {
    req.parseBody(this, iBuf, nbytes);
  }
  if (state == ClientState::READING_DONE) {
    LOG_TRACE("Request received from client fd:", fd);
    // handleRequest();
  }
}

/* void Client::handleRequest(void) {
  LOG_TRACE("Handling request from client fd:", fd);
  if (req.getMethod() == "GET") {
    LOG_INFO("GET request for path:", req.getReqURI());
    LOG_INFO("Request body:", req.getBody());
    LOG_INFO("Request headers:");
    for (auto& header : req.getHeaders()) {
      LOG_INFO(header.first, ":", header.second);
    }
  } else if (req.getMethod() == "POST") {
    LOG_INFO("POST request for path:", req.getReqURI());
    LOG_INFO("Request body:", req.getBody());
    LOG_INFO("Request headers:");
    for (auto& header : req.getHeaders()) {
      LOG_INFO(header.first, ":", header.second);
    }
  } else if (req.getMethod() == "DELETE") {
    LOG_INFO("DELETE request for path:", req.getReqURI());
    LOG_INFO("Request body:", req.getBody());
    LOG_INFO("Request headers:");
    for (auto& header : req.getHeaders()) {
      LOG_INFO(header.first, ":", header.second);
    }
  } else {
    LOG_ERROR("Unsupported method:", req.getMethod());
  }
} */

ServerConfig Client::chooseServerConfig(void) {
  LOG_TRACE("Choosing server config for client fd:", fd);
  for (auto& serverConfig : serverConfigs) {
    if (serverConfig->serverName == req.getHeaders()["Host"]) {
      return *serverConfig;
    }
  }
  return *(serverConfigs.front());
  // throw exception
}

bool Client::sendResponse(void) {
  if (state != ClientState::READING_DONE) {
    LOG_ERROR("Client", getFd(), "state is NOT done reading");
    return false;
  }
  res.setServerConfig(chooseServerConfig());
  LOG_TRACE("Creating response to client fd:", fd);
  res.run(req.getReqURI(), req.getMethod(), req.getBodySize());
  LOG_TRACE("Sending response");
  if (send(getFd(), res.getResContent().data(), res.getResContent().size(), 0) == -1) {
    LOG_ERROR("Failed to send response");
    return false;
  }
  state = ClientState::READING_REQLINE;
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
