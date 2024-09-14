#pragma once

#include <IServeAction.hpp>
#include <NetworkException.hpp>
#include <Response.hpp>

class Serve404Action : public IServeAction {
 public:
  Serve404Action() = default;

  virtual ~Serve404Action() = default;

  inline void execute(Response& res) override {
    throw httpForbidden(res, "HTTP Error 403 - Forbidden");
  }
};
