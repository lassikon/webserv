#pragma once

#include <IServeAction.hpp>

class Serve403Action : public IServeAction {
 public:
  Serve403Action() = default;

  virtual ~Serve403Action() = default;

  inline void execute(Client& client) override {
    throw httpForbidden(client, "HTTP Error 403 - Forbidden");
  }
};
