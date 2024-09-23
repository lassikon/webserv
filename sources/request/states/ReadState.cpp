#include <Client.hpp>
#include <ReadState.hpp>

void ReadState::execute(Client& client) {
  std::vector<char> buffer(4096, 0);
  ssize_t nbytes = read(client.getReadFd(), buffer.data(), buffer.size());
  client.setReadNBytes(nbytes);
  if (nbytes == -1) {
    LOG_WARN("Failed to receive data from client fd:", client.getFd());
    return;
  }

  if (nbytes == 0) {  // EOF
    handleEOF(client);
    return;
  }
  buffer[nbytes] = '\0';
  client.setReadBuf(std::make_shared<std::vector<char>>(buffer));
  LOG_INFO("Receiving data from fd",client.getReadFd(), "at", client.getFd());
  LOG_INFO("Received", nbytes, "bytes from client fd", client.getFd());
  LOG_INFO("Data received:", client.getReadBuf()->data());
  client.setParsingState(ParsingState::REQLINE);
  if (client.getReadFd() != client.getFd()) {
    LOG_DEBUG("Client", client.getFd(), "is CGI output");
    client.resetRequest();
    client.setParsingState(ParsingState::HEADER);
  }
  client.setClientState(ClientState::READING);
}

void ReadState::handleEOF(Client& client) {
  if (client.getReadBuf() == nullptr) {
    LOG_DEBUG("Reading EOF from client fd:", client.getFd());
    client.setClientState(ClientState::CLOSE);
    return;
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