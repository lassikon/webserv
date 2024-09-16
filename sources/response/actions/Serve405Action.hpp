#pragma once

#include <IServeAction.hpp>

class Serve405Action : public IServeAction {
 public:
  Serve405Action() = default;

  virtual ~Serve405Action() = default;

  inline void execute(Client& client) override {
    throw httpMethod(client, "HTTP Error 405 - Method not allowed");
  }
};
