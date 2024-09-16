#pragma once

#include <IServeAction.hpp>
class Client;
class Serve405Action : public IServeAction {
 public:
  Serve405Action() = default;
  virtual ~Serve405Action() = default;
  void execute(Client& client) override;
};
