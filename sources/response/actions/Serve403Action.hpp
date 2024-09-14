#pragma once

#include <IServeAction.hpp>
#include <NetworkException.hpp>
#include <Response.hpp>

class Serve403Action : public IServeAction {
 public:
  Serve403Action() = default;
  virtual ~Serve403Action() = default;

  inline void execute(Response& res) override {
    throw httpForbidden(res, "HTTP Error 403 - Forbidden");
  }
};
