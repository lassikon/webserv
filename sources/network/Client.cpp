#include <Client.hpp>
#include <NetworkException.hpp>

Client::Client(int socketFd, std::vector<std::shared_ptr<ServerConfig>>& serverConfigs)
    : fd(socketFd), serverConfigs(serverConfigs) {
  LOG_DEBUG(Utility::getConstructor(*this));
  clientState = ClientState::IDLE;
  cgiState = CgiState::IDLE;
  parsingState = ParsingState::IDLE;
}

Client::~Client(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
  cleanupClient();
}

bool Client::handleEpollEvents(uint32_t revents, int readFd, int writeFd) {
  if (revents & EPOLLIN) {
    NetworkException::tryCatch(&Client::handlePollInEvent, this, readFd);
  }
  if (revents & EPOLLOUT) {
    NetworkException::tryCatch(&Client::handlePollOutEvent, this, writeFd);
    if (shouldCloseConnection()) {
      return true;
    }
  }
  return false;
}

void Client::handlePollInEvent(int readFd) {
  LOG_INFO("Client fd:", fd, "has POLLIN event");
  ifGatewayError();
  setReadFd(readFd);
  if (clientState == ClientState::IDLE) {
    clientState = ClientState::READING;
  }
  if (clientState == ClientState::READING) {
    readState.execute(*this);
    parseState.execute(*this);
    //NetworkException::tryCatch(&ReadState::execute, &this->readState, *this);
    //NetworkException::tryCatch(&ParseState::execute, &this->parseState, *this);
  }
}

void Client::handlePollOutEvent(int writeFd) {
  LOG_INFO("Client fd:", fd, "has POLLOUT event");
  ifGatewayError();
  setWriteFd(writeFd);
  if (clientState == ClientState::PROCESSING) {
    processState.execute(*this);
    // NetworkException::tryCatch(&ProcessState::execute, &this->processState, *this);
  }
  if (clientState == ClientState::PREPARING) {
    LOG_DEBUG("Preparing response");
    if (writeNBytes == 0) {
      res.makeResponse();
    }
    clientState = ClientState::SENDING;
  }
  if (clientState == ClientState::SENDING) {
    sendState.execute(*this);
  }
  if (clientState == ClientState::DONE) {
    LOG_INFO("Client fd:", fd, "is done");
    LOG_DEBUG("Reinitializing client fd:", fd);
    initClient();
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
      throw clientError("Failed to close fd:", fd);
    }
    fd = -1;  // Mark as closed
  }
}

void Client::resetResponse(void) {
  res.setReqURI("");
  res.setResStatusCode(0);
  res.setResStatusMessage("");
  res.getResBody().clear();
  res.getResHeaders().clear();
  res.getResContent().clear();
}

void Client::resetRequest(void) {
  req.setQuery("");
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
  readBuf.reset();
  readBuf = nullptr;
  readCurr = 0;
  readEnd = 0;
  readNBytes = 0;
  writeNBytes = 0;
  if (clientState == ClientState::DONE) {
    clientState = ClientState::IDLE;
  }
  parsingState = ParsingState::IDLE;
  if (cgiState == CgiState::DONE) {
    cgiState = CgiState::IDLE;
    cgiHandler.closePipeFds();
  }
}

bool Client::shouldCloseConnection(void) {
  if (req.getHeaders().find("Connection") != req.getHeaders().end() &&
      req.getHeaders()["Connection"] == "close") {
    return true;
  }
  if (writeNBytes == -1 || res.getResStatusCode() == 500) {
    return true;
  }
  return false;
}

void Client::ifGatewayError(void) {
  for (auto& cgi : g_CgiParams) {
    if (cgi.clientFd == getFd() && cgi.isTimeout) {
      throw httpGatewayTimeout(*this, "CGI process timed out for client fd:", getFd());
    } else if (cgi.clientFd == getFd() && cgi.isFailed) {
      throw httpBadGateway(*this, "CGI process exited for client fd:", getFd());
    }
  }
}