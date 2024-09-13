#pragma once

#include <IRequestHandler.hpp>
#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>

class Client;

class GetHandler : public IRequestHandler {
 public:
  GetHandler() = default;
  virtual ~GetHandler() = default;
  void executeRequest(Client& client) override;

 private:
  std::shared_ptr<ProcessTree> root;
};