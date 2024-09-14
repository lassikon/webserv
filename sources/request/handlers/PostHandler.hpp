#pragma once

#include <IRequestHandler.hpp>

class Client;

class PostHandler : public IRequestHandler {
 public:
  PostHandler() = default;
  virtual ~PostHandler() = default;
  void executeRequest(Client& client) override;
};