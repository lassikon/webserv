#pragma once

#include <IRequestHandler.hpp>

class Client;
class ProcessTree;
class ProcessTreeBuilder;

class UploadHandler : public IRequestHandler {
 public:
  UploadHandler() = default;
  virtual ~UploadHandler() = default;
  void executeRequest(Client& client) override;

 private:
  std::shared_ptr<ProcessTree> root;
};
