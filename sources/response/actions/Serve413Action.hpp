#pragma once

#include <IServeAction.hpp>

class Serve413Action : public IServeAction {
 public:
  Serve413Action() = default;

  virtual ~Serve413Action() = default;

  inline void execute(Response& res) override {
    throw httpForbidden(res, "HTTP Error 403 - Forbidden");
  }
};
