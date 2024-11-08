#include <Client.hpp>
#include <NetworkException.hpp>
#include <SendState.hpp>

void SendState::execute(Client& client) {
  LOG_TRACE("Sending response");
  if (client.getRes().getResContent().empty()) {
    LOG_DEBUG("No data to send for client fd:", client.getFd());
    return;
  }
  ssize_t nbytes = 0;
  size_t size = client.getRes().getResContent().size() - client.getWriteNBytes();
  auto buffer = client.getRes().getResContent().data() + client.getWriteNBytes();
  nbytes = write(client.getWriteFd(), buffer, size);
  client.setWriteNBytes(client.getWriteNBytes() + nbytes);

  if (nbytes == -1) {  // write error
    client.setCloseConnection(true);
    LOG_ERROR("Failed to write to client fd:", client.getFd(), IException::expandErrno());
    return;
  }

  if (nbytes == 0) {  // received EOF
    LOG_DEBUG("Write of EOF for client fd:", client.getFd());
    client.setClientState(ClientState::DONE);
    if (client.getCgiState() == CgiState::READING) {
      client.setCgiState(CgiState::WRITING);
      client.setClientState(ClientState::READING);
    }
    if (client.getRes().getResStatusCode() >= 400) {
      LOG_DEBUG("closing connection for client fd:", client.getFd());
      client.setCloseConnection(true);
    }
    client.initClient();
    return;
  }

  LOG_DEBUG("bytes sent:", nbytes);
  LOG_DEBUG("total bytes:", client.getRes().getResContent().size());
  LOG_DEBUG("Response sent to fd:", client.getWriteFd(), " from fd:", client.getFd());
  LOG_DEBUG("response content:", client.getRes().getResContent().data());
  if (client.getRes().getResStatusCode() < 400 && client.getRes().getResStatusCode() != 0) {
    LOG_ANNOUNCE("Client fd:", client.getFd(), "response:", client.getRes().getResStatusCode(),
                 client.getRes().getResStatusMessage());
  } else if (client.getRes().getResStatusCode() >= 400) {
    LOG_WARN("Client fd:", client.getFd(), "response:", client.getRes().getResStatusCode(),
             client.getRes().getResStatusMessage());
  }
  client.setClientState(ClientState::SENDING);
}
