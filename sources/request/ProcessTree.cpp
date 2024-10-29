#include <Client.hpp>
#include <NetworkException.hpp>
#include <ProcessTree.hpp>

ProcessTree::ProcessTree(Validate validate, std::shared_ptr<ProcessTree> trueBranch,
                         std::shared_ptr<ProcessTree> falseBranch)
    : validate(validate), trueBranch(trueBranch), falseBranch(falseBranch) {}

ProcessTree::ProcessTree(std::shared_ptr<IServeAction> action) : action(action) {}

ProcessTree::ProcessTree(int statusCode) : statusCode(statusCode) {}

ProcessTree::~ProcessTree() {}

void ProcessTree::process(Client& client) {
  if (validate) {
    if (validate(client.getRes().getReqURI())) {
      LOG_TRACE("ProcessTree: true branch");
      if (trueBranch)
        trueBranch->process(client);
    } else {
      LOG_TRACE("ProcessTree: false branch");
      if (falseBranch)
        falseBranch->process(client);
    }
  } else {
    LOG_TRACE("ProcessTree: action");
    if (action)
      action->execute(client);
    else
      thorwError(client);
  }
}

void ProcessTree::thorwError(Client& client) {
  switch (statusCode) {
    case 400:
      throw httpBadRequest(client, "HTTP Error 400 - Bad Request");
      break;
    case 403:
      throw httpForbidden(client, "HTTP Error 403 - Forbidden");
      break;
    case 404:
      throw httpNotFound(client, "HTTP Error 404 - Not Found");
      break;
    case 405:
      throw httpMethod(client, "HTTP Error 405 - Method Not Allowed");
      break;
    case 411:
      throw httpLength(client, "HTTP Error 411 - Length Required");
      break;
    case 413:
      throw httpPayload(client, "HTTP Error 413 - Payload Too Large");
      break;
    case 500:
      throw httpInternal(client, "HTTP Error 500 - Internal Server Error");
      break;
    case 502:
      throw httpBadGateway(client, "HTTP Error 502 - Bad Gateway");
      break;
    case 504:
      throw httpGatewayTimeout(client, "HTTP Error 504 - Gateway Timeout");
      break;
    default:
      break;
  }
}