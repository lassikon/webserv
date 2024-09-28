#include <Client.hpp>
#include <ReadState.hpp>

void ReadState::execute(Client& client) {
  std::vector<char> buffer(4096, 0);
  ssize_t nbytes = 0;
  nbytes = read(client.getReadFd(), buffer.data(), buffer.size());
  client.setReadNBytes(nbytes + client.getReadNBytes());
  if (nbytes == -1) {
    LOG_WARN("Failed to receive data from client fd:", client.getFd());
    return;
  }
  if (nbytes == 0) {  // EOF
    handleIfEOF(client);
    return;
  }
  handleReadBuf(client, buffer, nbytes);
  LOG_INFO("Receiving data from fd", client.getReadFd(), "at", client.getFd());
  LOG_INFO("Received", nbytes, "bytes from client fd", client.getFd());
  LOG_INFO("Data received:", buffer.data());
  isCRLF(client);
  handleIfCgiOutput(client);
}

void ReadState::handleReadBuf(Client& client, std::vector<char> buffer, ssize_t nbytes) {
  client.setReadNBytes(nbytes + client.getReadNBytes());
  std::shared_ptr<std::vector<char>> temp = std::make_shared<std::vector<char>>(nbytes);
  temp->assign(buffer.begin(), buffer.begin() + nbytes);
  if (client.getReadBuf() != nullptr) {
    LOG_TRACE("APPENDING before size:", client.getReadBuf()->size());
    std::vector<char> buf = *client.getReadBuf();
    buf.insert(buf.end(), temp->begin(), temp->end());
    client.setReadBuf(std::make_shared<std::vector<char>>(buf));
    client.setReadEnd(client.getReadBuf()->end());
    LOG_TRACE("APPENDING after size:", client.getReadBuf()->size());
  } else {
    client.setReadBuf(std::make_shared<std::vector<char>>(temp->begin(), temp->end()));
    client.setReadIt(client.getReadBuf()->begin());
    client.setReadEnd(client.getReadBuf()->end());
  }
}

void ReadState::isCRLF(Client& client) {
  LOG_DEBUG("Finding CRLF");
  std::vector<char> clrf = {'\r', '\n', '\r', '\n'};
  auto pos =
    std::search(client.getReadBuf()->begin(), client.getReadBuf()->end(), clrf.begin(), clrf.end());
  if (pos != client.getReadBuf()->end()) {
    LOG_DEBUG("CRLF found");
    client.setParsingState(ParsingState::REQLINE);
  }
}

void ReadState::handleIfCgiOutput(Client& client) {
  if (client.getReadFd() != client.getFd()) {
    LOG_DEBUG("Client", client.getFd(), "is CGI output");
    client.resetRequest();
    client.setParsingState(ParsingState::HEADER);
  }
}

void ReadState::handleIfEOF(Client& client) {
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
  }
}