#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

class Client;

class ProcessTree {
 public:
  using Validate = std::function<bool(std::string& path)>;
  ProcessTree(Validate validate, std::shared_ptr<ProcessTree> trueBranch,
              std::shared_ptr<ProcessTree> falseBranch);
  ProcessTree(std::shared_ptr<IServeAction> action);
  ProcessTree(int statusCode);
  ~ProcessTree();
  void process(Client& client);

 private:
  Validate validate = nullptr;
  std::shared_ptr<ProcessTree> trueBranch = nullptr;
  std::shared_ptr<ProcessTree> falseBranch = nullptr;
  std::shared_ptr<IServeAction> action = nullptr;
  int statusCode = 0;
  void thorwError(Client& client);
};
