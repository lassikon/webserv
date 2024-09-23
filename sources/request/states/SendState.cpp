#include <Client.hpp>
#include <SendState.hpp>

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
    LOG_ERROR("Failed to send response");
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
  LOG_INFO("Response sent to fd:", client.getWriteFd(), " from fd:", client.getFd());
  LOG_INFO("Response:", client.getRes().getResContent().data());
  client.setClientState(ClientState::SENDING);
}
