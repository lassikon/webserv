#include <Client.hpp>

Client::Client(int socketFd, std::vector<std::shared_ptr<ServerConfig>>& serverConfigs)
    : fd(socketFd), serverConfigs(serverConfigs) {
  LOG_DEBUG(Utility::getConstructor(*this));
  clientState = ClientState::READING;
}

Client::~Client(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
  cleanupClient();
}

bool Client::operator==(const Client& other) const {
  return fd == other.fd;
}

bool Client::handlePollEvents(short revents, int readFd, int writeFd) {
  if (revents & POLLIN) {
    handlePollInEvent(readFd);
  }
  if (revents & POLLOUT) {
    handlePollOutEvent(writeFd);
    if (shouldCloseConnection()) {
      return false;
    }
  }
  return true;
}

void Client::handlePollInEvent(int readFd) {
  LOG_INFO("Client fd:", fd, "has POLLIN event");
  setReadFd(readFd);
  if (clientState == ClientState::READING) {
    readState.execute(*this);
    parseState.execute(*this);
  }
}

void Client::handlePollOutEvent(int writeFd) {
  LOG_INFO("Client fd:", fd, "has POLLOUT event");
  setWriteFd(writeFd);
  if (clientState == ClientState::PROCESSING) {
    try {
      processState.execute(*this);
    } catch (HttpException& e) {
      LOG_ERROR("Exception caught:", e.what());
      e.setResponseAttributes();
      clientState = ClientState::SENDING;
    }
  }
  if (clientState == ClientState::SENDING) {
    res.makeResponse();
    sendState.execute(*this);
  }
  if (clientState == ClientState::DONE) {
    LOG_INFO("Client fd:", fd, "is done");
    LOG_DEBUG("Cleaning up client fd:", fd);
    initClient();
    clientState = ClientState::READING;
  }
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

void Client::resetResponse(void) {
  res.setReqURI("");
  res.setResStatusCode(0);
  res.setResStatusMessage("");
  std::vector<char> body = {};
  res.setResBody(body);
  res.getResHeaders().clear();
  res.getResContent().clear();
}

void Client::resetRequest(void) {
  req.setMethod("");
  req.setReqURI("");
  req.setVersion("");
  req.getHeaders().clear();
  req.getBody().clear();
  req.setBodySize(0);
}

void Client::initClient(void) {
  resetRequest();
  resetResponse();
  isCgi = false;
  readBuf = nullptr;
  readNBytes = 0;
  writeNBytes = 0;
  clientState = ClientState::READING;
  parsingState = ParsingState::REQLINE;
}

bool Client::shouldCloseConnection(void) {
  if (req.getHeaders().find("Connection") != req.getHeaders().end() &&
      req.getHeaders()["Connection"] == "close") {
    return true;
  }
  if (writeNBytes == -1) {
    return true;
  }
  return false;
}
