#pragma once

#include <IServeAction.hpp>

class Serve413Action : public IServeAction {
 public:
  Serve413Action() = default;

  virtual ~Serve413Action() = default;

  inline void execute(Client& client) override {
    throw httpPayload(client, "HTTP Error 413 - Payload too large");
  }
};
