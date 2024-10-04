#pragma once

#include <IRequestHandler.hpp>

class Client;

class DeleteHandler : public IRequestHandler {
 public:
  DeleteHandler() = default;
  virtual ~DeleteHandler() {LOG_DEBUG(Utility::getDeconstructor(*this));};
  void executeRequest(Client& client) override;
};