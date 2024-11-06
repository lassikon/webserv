#include <Client.hpp>
#include <SendState.hpp>
#include <NetworkException.hpp>

void SendState::execute(Client& client) {
  if (client.getRes().getResContent().empty()) {
    LOG_TRACE("No data to send for client fd:", client.getFd());
    return;
  }

  LOG_TRACE("Sending response");
  ssize_t nbytes = 0;
  nbytes =
    write(client.getWriteFd(), client.getRes().getResContent().data() + client.getWriteNBytes(),
          client.getRes().getResContent().size() - client.getWriteNBytes());
  client.setWriteNBytes(client.getWriteNBytes() + nbytes);
  if (nbytes == -1) {
    client.setCloseConnection(true);
    LOG_ERROR("Failed to write to client fd:", client.getFd(), IException::expandErrno(), "(", errno, ")");
    return;
  }

  if (nbytes == 0) {
    LOG_TRACE("Write of EOF for client fd:", client.getFd());
    client.setClientState(ClientState::DONE);
    if (client.getCgiState() == CgiState::READING) {
      client.setCgiState(CgiState::WRITING);
      client.setClientState(ClientState::READING);
    }
    client.initClient();
    return;
  }

  LOG_DEBUG("bytes sent:", nbytes);
  LOG_DEBUG("total bytes:", client.getRes().getResContent().size());
  LOG_DEBUG("Response sent to fd:", client.getWriteFd(), " from fd:", client.getFd());
  LOG_ANNOUNCE("response content:", client.getRes().getResContent().data());
  if (client.getRes().getResStatusCode() < 400 && client.getRes().getResStatusCode() != 0) {
    LOG_ANNOUNCE("Client fd:", client.getFd(), "response:", client.getRes().getResStatusCode(), client.getRes().getResStatusMessage());
  }
  else if (client.getRes().getResStatusCode() >= 400) {
    LOG_WARN("Client fd:", client.getFd(), "response:", client.getRes().getResStatusCode(), client.getRes().getResStatusMessage());
  }
  client.setClientState(ClientState::SENDING);
}
