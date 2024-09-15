#pragma once

#include <IServeAction.hpp>

class Serve405Action : public IServeAction {
 public:
  Serve405Action() = default;

  virtual ~Serve405Action() = default;

  inline void execute(Response& res) override {
    throw httpForbidden(res, "HTTP Error 403 - Forbidden");
  }
};
