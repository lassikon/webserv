#pragma once

#include <IRequestHandler.hpp>

class Client;

class DeleteHandler : public IRequestHandler {
 public:
  DeleteHandler() = default;
  virtual ~DeleteHandler() = default;
  void executeRequest(Client& client) override;
};