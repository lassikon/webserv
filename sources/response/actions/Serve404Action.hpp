#pragma once

#include <IServeAction.hpp>

class Client;

class Serve404Action : public IServeAction {
 public:
  Serve404Action() = default;
  virtual ~Serve404Action() = default;
  void execute(Client& client) override;
};
