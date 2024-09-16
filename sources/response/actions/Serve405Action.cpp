#include <NetworkException.hpp>
#include <Serve405Action.hpp>

void Serve405Action::execute(Client& client) {
  throw httpMethod(client, "HTTP Error 405 - Method not allowed");
}