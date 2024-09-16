#include <NetworkException.hpp>
#include <Serve404Action.hpp>

void Serve404Action::execute(Client& client) {
  throw httpNotFound(client, "HTTP Error 404 - Page not found");
}