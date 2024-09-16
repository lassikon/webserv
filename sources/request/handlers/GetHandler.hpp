#pragma once

#include <IRequestHandler.hpp>

class Client;
class ProcessTree;
class ProcessTreeBuilder;

class GetHandler : public IRequestHandler {
 public:
  GetHandler() = default;
  virtual ~GetHandler() = default;
  void executeRequest(Client& client) override;

 private:
  std::shared_ptr<ProcessTree> root;
};
