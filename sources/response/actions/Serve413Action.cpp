#include <NetworkException.hpp>
#include <Serve413Action.hpp>

void Serve413Action::execute(Client& client) {
  throw httpPayload(client, "HTTP Error 413 - Payload too large");
}