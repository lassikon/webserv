#include <Client.hpp>
#include <ReadState.hpp>

void ReadState::execute(Client& client) {
  char buffer[4096] = {0};
  ssize_t nbytes = read(client.getReadFd(), buffer, sizeof(buffer));
  client.setReadNBytes(nbytes);
  if (nbytes == -1) {
    LOG_WARN("Failed to receive data from client fd:", client.getFd());
    return;
  }
  if (nbytes == 0) {  // EOF
    LOG_TRACE("Read of EOF for client fd:", client.getFd());
    if (client.getReadBuf() == nullptr) {
      LOG_DEBUG("Client fd:", client.getFd(), "is done");
      client.setClientState(ClientState::DONE);
      return;
    }
    LOG_DEBUG("Client fd:", client.getFd(), "is CGI EOF");
    client.setParsingState(ParsingState::DONE);
    client.setClientState(ClientState::PROCESSING);
    return;
  }
  client.setReadBuf(buffer);
  LOG_INFO("Receiving data from client fd", client.getFd());
  LOG_INFO("Received", nbytes, "bytes from client fd", client.getFd());
  LOG_INFO("Data received:", client.getReadBuf());
  client.setParsingState(ParsingState::REQLINE);
  if (client.getIsCgi()) {
    LOG_DEBUG("Client", client.getFd(), "is CGI output");
    client.resetRequest();
    client.setParsingState(ParsingState::HEADER);
  }
}

/* bool ReadState::isCgiOutput(Client& client, std::string buf) {
  if (buf.find("HTTP/1.1") != std::string::npos) {
    client.setIsCgi(false);
    return false;
  }
  client.setIsCgi(true);
  return true;
} */