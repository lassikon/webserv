#include <Client.hpp>
#include <SendState.hpp>

void SendState::execute(Client& client) {
  if (client.getRes().getResContent().empty()) {
    LOG_TRACE("No data to send for client fd:", client.getFd());
    return;
  }
  LOG_TRACE("Sending response");
  ssize_t nbytes;
  nbytes = send(client.getWriteFd(), client.getRes().getResContent().data(),
                client.getRes().getResContent().size(), 0);
  client.setWriteNBytes(nbytes);
  if (nbytes == -1) {
    LOG_ERROR("Failed to send response");
    return;
  }

  LOG_DEBUG("bytes sent:", nbytes);
  LOG_DEBUG("total bytes:", client.getRes().getResContent().size());
  LOG_INFO("Response sent to client fd:", client.getFd());
  LOG_INFO("Response:", client.getRes().getResContent().data());
  client.setClientState(ClientState::DONE);
}
