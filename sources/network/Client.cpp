#include <Client.hpp>

Client::Client(int socketFd, std::vector<std::shared_ptr<ServerConfig>>& serverConfigs)
    : fd(socketFd), serverConfigs(serverConfigs) {
  LOG_TRACE(Utility::getConstructor(*this));
  state = ClientState::READING_REQLINE;
}

Client::~Client(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  cleanupClient();
}

bool Client::operator==(const Client& other) const {
  return fd == other.fd;
}

bool Client::handlePollEvents(short revents, int readFd, int writeFd) {
  if (revents & POLLIN) {
    LOG_INFO("Client fd:", fd, "has POLLIN event");
    if (!receiveData(readFd)) {
      return false;
    }
  }
  if (revents & POLLOUT) {
    LOG_INFO("Client fd:", fd, "has POLLOUT event");
    if (!handleRequest(writeFd)) {
      return false;
    }
  }
  return true;
}

bool Client::receiveData(int readFd) {
  char buf[4096] = {0};
  ssize_t nbytes = read(readFd, buf, sizeof(buf));
  if (nbytes == -1) {
    LOG_WARN("Failed to receive data from client fd:", fd);
  } else if (nbytes == 0) {  // Connection closed
    LOG_TRACE("Connection closed for client fd:", fd);
    return false;
  } else {
    LOG_INFO("Receiving data from client fd", fd);
    if (isCgiOutput(std::string(buf))) {
      state = ClientState::READING_HEADER;
      LOG_DEBUG("Client", fd, "is CGI output");
      Request reqCGI;
      req = reqCGI;
    }
    std::istringstream iBuf(buf);
    parseRequest(iBuf, nbytes);
  }
  return true;
}

void Client::parseRequest(std::istringstream& iBuf, int nbytes) {
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
  }
}

bool Client::handleRequest(int writeFd) {
  if (state != ClientState::READING_DONE) {
    LOG_WARN("Client", getFd(), "state is processing");
    return true;
  }
  LOG_TRACE("Handling request from client fd:", fd);
  if (isCgi) {
    processCgiOutput();
  } else {
    NetworkException::tryCatch(&Client::processRequest, this);
  }
  if (state == ClientState::DONE) {
    if (!sendResponse(writeFd)) {
      return false;
    }
  }
  return true;
}

void Client::processCgiOutput(void) {
  res.setResStatusCode(200);
  res.setResStatusMessage("OK");
  if (req.getHeaders().find("Status") != req.getHeaders().end()) {
    std::string statusLine = req.getHeaders()["Status"];
    res.setResStatusCode(std::stoi(statusLine.substr(0, 3)));
    res.setResStatusMessage(statusLine.substr(4));
  }
  for (auto& header : req.getHeaders()) {
    if (header.first != "Status") {
      res.addHeader(header.first, header.second);
    }
  }
  res.addHeader("Connection", "keep-alive");
  res.addHeader("Content-Length", std::to_string(req.getBodySize()));
  std::vector<char> body(req.getBodySize());
  std::copy(req.getBody().begin(), req.getBody().end(), body.begin());
  res.setResBody(body);
  res.makeResponse();
  state = ClientState::DONE;
}

void Client::buildPath(void) {
  LOG_TRACE("Building path for client fd:", fd);
  std::shared_ptr<ProcessTreeBuilder> ptb =
    std::make_shared<ProcessTreeBuilder>(req, res, res.getServerConfig());
  res.setReqURI(req.getReqURI());
  root = ptb->buildPathTree();
  root->process(res);
}

void Client::processRequest(void) {
  res.setServerConfig(chooseServerConfig());
  buildPath();
  if (res.getReqURI().find("/cgi-bin/") != std::string::npos) {
    cgiHandler.executeRequest(*this);
    state = ClientState::READING_REQLINE;
    return;
  } else if (req.getMethod() == "GET") {
    getHandler.executeRequest(*this);
  } else if (req.getMethod() == "POST") {
    postHandler.executeRequest(*this);
  } else if (req.getMethod() == "DELETE") {
    deleteHandler.executeRequest(*this);
  } else {
    LOG_ERROR("Unsupported method in client:", fd);
  }
  res.makeResponse();
  state = ClientState::DONE;
}

ServerConfig Client::chooseServerConfig(void) {
  LOG_TRACE("Choosing server config for client fd:", fd);
  for (auto& serverConfig : serverConfigs) {
    if (serverConfig->serverName == req.getHeaders()["Host"]) {
      return *serverConfig;
    }
  }
  return *(serverConfigs.front());
}

bool Client::sendResponse(int writeFd) {
  LOG_TRACE("Sending response");
  if (res.getResContent().empty()) {
    LOG_DEBUG("No data to send for client fd:", fd);
    return true;
  }
  ssize_t nbytes;
  nbytes = send(writeFd, res.getResContent().data(), res.getResContent().size(), 0);
  if (nbytes == -1) {
    LOG_ERROR("Failed to send response");
    return false;
  }
  LOG_DEBUG("Sent:", nbytes, "bytes, ", res.getResContent().size(), "in total");
  LOG_INFO("Response sent to client fd:", fd);
  state = ClientState::READING_REQLINE;
  if (req.getHeaders().find("Connection") != req.getHeaders().end() &&
      req.getHeaders()["Connection"] == "close") {
    LOG_DEBUG("Client", fd, "request to close connection");
    return false;
  }
  resetResponse();
  return true;
}

bool Client::isCgiOutput(std::string buf) {
  if (buf.find("HTTP/1.1") != std::string::npos) {
    return false;
  }
  isCgi = true;
  return true;
}

void Client::setClientFd(int newFd) {
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
      throw clientError("Failed to close fd:", fd);
    }
    closeClientFd();
  }
}

void Client::resetResponse(void) {
  std::vector<char> body = {};
  res.setResBody(body);
  res.setResStatusCode(0);
  res.setResStatusMessage("");
  res.getResHeaders().clear();
  res.getResContent().clear();
  res.setReqURI("");
}
