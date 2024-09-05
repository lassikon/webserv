#include <ProcessTree.hpp>

ProcessTree::ProcessTree(Validate validate,
                         std::shared_ptr<ProcessTree> trueBranch,
                         std::shared_ptr<ProcessTree> falseBranch)
    : validate(validate), trueBranch(trueBranch), falseBranch(falseBranch) {}

ProcessTree::ProcessTree(Action action) : action(action) {}

ProcessTree::~ProcessTree() {}

void ProcessTree::process(std::string& path) {
  if (validate) {
    if (validate(path)) {
      LOG_TRACE("ProcessTree: true branch");
      trueBranch->process(path);
    } else {
      LOG_TRACE("ProcessTree: false branch");
      falseBranch->process(path);
    }
  } else {
    action(path);
  }
}