#pragma once

#include <IStateHandler.hpp>
#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>

class Client;

class ProcessState : public IStateHandler {
 public:
  ProcessState() = default;
  virtual ~ProcessState() = default;
  void execute(Client& client) override;

 private:
  std::shared_ptr<ProcessTree> root;

 private:
  void processCgiOutput(Client& client);
  void processRequest(Client& client);
  ServerConfig chooseServerConfig(Client& client);
  void buildPath(Client& client);
  bool isWithStatusCode(Client& client);
};
