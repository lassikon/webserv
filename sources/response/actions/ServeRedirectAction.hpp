#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>

class Client;

class ServeRedirectAction : public IServeAction {
 public:
  ServeRedirectAction() = default;
  virtual ~ServeRedirectAction() = default;
  void execute(Client& client) override;
};
