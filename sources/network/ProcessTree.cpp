#include <ProcessTree.hpp>

ProcessTree::ProcessTree(Validate validate,
                         std::shared_ptr<ProcessTree> trueBranch,
                         std::shared_ptr<ProcessTree> falseBranch)
    : validate(validate), trueBranch(trueBranch), falseBranch(falseBranch) {
  LOG_TRACE("ProcessTree decision constructor called");
  if (!trueBranch || !falseBranch) {
    throw std::invalid_argument("trueBranch and falseBranch must be non-null");
  }
  if (validate) {
    LOG_TRACE("ProcessTree decision constructor with validate");
  }
}

ProcessTree::ProcessTree(Action action) : action(action) {
  LOG_TRACE("ProcessTree serverconstructor called");
}

ProcessTree::~ProcessTree() { LOG_TRACE("ProcessTree destructor called"); }

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