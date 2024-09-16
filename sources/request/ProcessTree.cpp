#include <ProcessTree.hpp>
#include <Client.hpp>
ProcessTree::ProcessTree(Validate validate, std::shared_ptr<ProcessTree> trueBranch,
                         std::shared_ptr<ProcessTree> falseBranch)
    : validate(validate), trueBranch(trueBranch), falseBranch(falseBranch) {}

ProcessTree::ProcessTree(std::shared_ptr<IServeAction> action) : action(action) {}

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
  }
}