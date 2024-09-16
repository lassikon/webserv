#include <Client.hpp>
#include <ServeRedirectAction.hpp>

void ServeRedirectAction::execute(Client& client) {
  LOG_TRACE("ServeRedirectAction execute called");
  LOG_DEBUG("Redirecting to: ", client.getRes().getRouteConfig().redirect);
  client.getRes().setResStatusCode(301);
  client.getRes().setResStatusMessage("Moved Permanently");
  client.getRes().addHeader("Location", client.getRes().getRouteConfig().redirect);
  client.getRes().addHeader("Connection", "keep-alive");
}
