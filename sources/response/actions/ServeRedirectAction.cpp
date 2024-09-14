#include <ServeRedirectAction.hpp>


void ServeRedirectAction::execute(Response& res) {
  LOG_TRACE("ServeRedirectAction execute called");
  LOG_DEBUG("Redirecting to: ", res.getRouteConfig().redirect);
  res.setResStatusCode(301);
  res.setResStatusMessage("Moved Permanently");
  res.addHeader("Location", res.getRouteConfig().redirect);
  res.addHeader("Connection", "keep-alive");
}
