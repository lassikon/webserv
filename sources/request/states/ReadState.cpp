#include <Client.hpp>
#include <NetworkException.hpp>
#include <ReadState.hpp>

void ReadState::execute(Client& client) {
  std::vector<char> buffer(4096, 0);
  ssize_t nbytes = 0;
  LOG_DEBUG("hllo ", client.getReadFd());
  nbytes = read(client.getReadFd(), buffer.data(), buffer.size());
  client.setReadNBytes(nbytes + client.getReadNBytes());
  if (nbytes == -1) {
    throw httpInternal(client, "Failed to read from client fd:", client.getFd());
  }
  if (nbytes == 0) {  // EOF
    handleEOF(client);
    return;
  }
  handleReadBuf(client, buffer, nbytes);
  LOG_DEBUG("Receiving data from fd", client.getReadFd(), "at", client.getFd());
  LOG_DEBUG("Received", nbytes, "bytes from client fd", client.getFd());
  LOG_DEBUG("Data received:", buffer.data());
  ifCRLF(client);
  ifCgiOutput(client);
}

void ReadState::handleReadBuf(Client& client, std::vector<char> buffer, ssize_t nbytes) {
  client.setReadNBytes(nbytes + client.getReadNBytes());
  std::shared_ptr<std::vector<char>> temp = std::make_shared<std::vector<char>>(nbytes);
  temp->assign(buffer.begin(), buffer.begin() + nbytes);
  if (client.getReadBuf() != nullptr) {
    LOG_DEBUG("APPENDING before size:", client.getReadBuf()->size());
    std::vector<char>& buf = *client.getReadBuf();
    buf.insert(buf.end(), temp->begin(), temp->end());
    client.setReadBuf(std::move(buf));
    client.setReadEnd(client.getReadBuf()->size());
    LOG_DEBUG("APPENDING after size:", client.getReadBuf()->size());
  } else {
    LOG_TRACE("CREATING new buffer");
    client.setReadBuf(std::make_shared<std::vector<char>>(temp->begin(), temp->end()));
    client.setReadCurr(0);
    client.setReadEnd(client.getReadBuf()->size());
  }
}

void ReadState::ifCRLF(Client& client) {
  if (client.getParsingState() != ParsingState::IDLE) {
    return;
  }
  LOG_TRACE("Finding CRLF");
  std::vector<char> clrf = {'\r', '\n', '\r', '\n'};
  auto pos =
    std::search(client.getReadBuf()->begin(), client.getReadBuf()->end(), clrf.begin(), clrf.end());
  if (pos != client.getReadBuf()->end()) {
    LOG_TRACE("CRLF found");
    LOG_DEBUG("Read curr:", client.getReadCurr());
    LOG_DEBUG("Read end:", client.getReadEnd());
    client.setParsingState(ParsingState::REQLINE);
  }
}

void ReadState::ifCgiOutput(Client& client) {
  if (client.getReadFd() != client.getFd()) {
    LOG_DEBUG("Client", client.getFd(), "is CGI output");
    client.resetRequest();
    client.setParsingState(ParsingState::HEADER);
  }
}

void ReadState::handleEOF(Client& client) {
  if (client.getReadBuf() == nullptr) {
    LOG_DEBUG("Reading EOF from client fd:", client.getFd());
    client.setClientState(ClientState::CLOSE);
  } else if (client.getCgiState() != CgiState::IDLE) {
    LOG_DEBUG("Client fd:", client.getFd(), "is CGI EOF");
    client.setParsingState(ParsingState::DONE);
    client.setClientState(ClientState::PROCESSING);
    client.setCgiState(CgiState::DONE);
  } else if (client.getReadBuf()->empty()) {
    LOG_DEBUG("Reading EOF from client fd:", client.getFd());
    client.setClientState(ClientState::PROCESSING);
  } else {
    LOG_DEBUG("Reading EOF from client fd:", client.getFd());
    throw httpBadRequest(client, "Client fd:", client.getFd(), "has unexpected EOF");
  }
}
