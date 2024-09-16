#include <Serve403Action.hpp>
#include <NetworkException.hpp>

void Serve403Action::execute(Client& client) {
  throw httpForbidden(client, "HTTP Error 403 - Forbidden");
}