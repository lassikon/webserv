#include <ProcessTree.hpp>

ProcessTree::ProcessTree(Validate validate, std::shared_ptr<ProcessTree> trueBranch,
                         std::shared_ptr<ProcessTree> falseBranch)
    : validate(validate), trueBranch(trueBranch), falseBranch(falseBranch) {}

ProcessTree::ProcessTree(std::shared_ptr<IServeAction> action) : action(action) {}

ProcessTree::~ProcessTree() {}

void ProcessTree::process(Response& res) {
  if (validate) {
    if (validate(res.getReqURI())) {
      LOG_TRACE("ProcessTree: true branch");
      if (trueBranch)
        trueBranch->process(res);
    } else {
      LOG_TRACE("ProcessTree: false branch");
      if (falseBranch)
        falseBranch->process(res);
    }
  } else {
    LOG_TRACE("ProcessTree: action");
    if (action)
      action->execute(res);
  }
}