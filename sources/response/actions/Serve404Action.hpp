#pragma once

#include <IServeAction.hpp>
#include <NetworkException.hpp>
#include <Client.hpp>
class Serve404Action : public IServeAction {
 public:
  Serve404Action() = default;

  virtual ~Serve404Action() = default;

  inline void execute(Client& client) override {
    throw httpNotFound(client, "HTTP Error 404 - Page not found");
  }
};
