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
    LOG_INFO("Client fd:", fd, "has POLLIN event");
    if (!receiveData()) {
      return false;
    }
  }
  if (revents & POLLOUT) {
    LOG_INFO("Client fd:", fd, "has POLLOUT event");
    if (!handleRequest()) {
      return false;
    }
  }
  return true;
}

bool Client::receiveData(void) {
  char buf[4096] = {0};
  ssize_t nbytes = recv(fd, buf, sizeof(buf), 0);
  if (nbytes == -1) {
    clientError("Failed to recv() from fd:", fd);
  } else if (nbytes == 0) {  // Connection closed
    LOG_TRACE("Connection closed for client fd:", fd);
    return false;
  } else {
    LOG_INFO("Receiving data from client fd", fd, ", buffer:", buf);
    std::istringstream bufStr(buf);
    parseRequest(bufStr, nbytes);
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

bool Client::handleRequest(void) {
  if (state != ClientState::READING_DONE) {
    LOG_ERROR("Client", getFd(), "state is NOT done reading");
    return false;
  }
  LOG_TRACE("Handling request from client fd:", fd);
  processRequest();
  if (!sendResponse()) {
    return false;
  }
  return true;
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
  res.setServerConfig(chooseServerConfig());  // choose server config
  try {
    buildPath();
    if (res.getReqURI().find("/cgi-bin/") != std::string::npos) {
      cgiHandler.executeRequest(req, res);
      res.setResStatusCode(200);
      res.setResStatusMessage("OK");
      char buffer[4096] = {0};
      LOG_DEBUG("Reading from cgiFd:", res.getCgiFd());
      read(res.getCgiFd(), &buffer, sizeof(buffer));
      std::cout << "here buffer: " << buffer << std::endl;
      auto body = std::vector<char>(buffer, buffer + sizeof(buffer));
      std::cout << "here body: " << body.data() << std::endl;
      res.addHeader("Content-Type", "text/html");
      res.addHeader("Content-Length", std::to_string(body.size()));
      res.addHeader("Server", res.getServerConfig().serverName);
      res.addHeader("Connection", "close");
      res.setResBody(body);
    } else if (req.getMethod() == "GET") {
      getHandler.executeRequest(req, res);
    } else if (req.getMethod() == "POST") {
      postHandler.executeRequest(req, res);
    } else if (req.getMethod() == "DELETE") {
      deleteHandler.executeRequest(req, res);
    } else if (req.getMethod() == "GET") {
      getHandler.executeRequest(req, res);
    } else {
      LOG_ERROR("Unsupported method in client:", fd);
    }
  } catch (HttpException& e) {
    LOG_ERROR("Exception caught:", e.what());
    e.setResponseAttributes();
  }
  res.makeResponse();
}

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
  LOG_TRACE("Sending response");
  ssize_t nbytes;
  nbytes = send(getFd(), res.getResContent().data(), res.getResContent().size(), 0);
  if (nbytes == -1) {
    LOG_ERROR("Failed to send response");
    return false;
  }
  LOG_DEBUG("bytes sent:", nbytes);
  LOG_DEBUG("total bytes:", res.getResContent().size());
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
