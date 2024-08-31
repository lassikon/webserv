#pragma once

#include <iostream>
#include <string>
#include <functional>
#include <memory>

#include <Logger.hpp>

class ProcessTree {

 public:
    using Validate = std::function<bool(std::string& path)>;
    using Action = std::function<void(std::string& path)>;
    ProcessTree(Validate validate, std::shared_ptr<ProcessTree> trueBranch,
                std::shared_ptr<ProcessTree> falseBranch);
    ProcessTree(Action action);
    ~ProcessTree();
    void process(std::string& path);

 private:
    Validate validate;
    Action action;
    std::shared_ptr<ProcessTree> trueBranch;
    std::shared_ptr<ProcessTree> falseBranch;
};