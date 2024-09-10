#pragma once

#include <IServeAction.hpp>
#include <HttpException.hpp>
#include <Forbidden.hpp>
#include <Response.hpp>
#include <Logger.hpp>

class Serve403Action : public IServeAction {
 public:
  Serve403Action() = default;
  virtual ~Serve403Action() = default;
  inline void execute(Response& res) override {
    LOG_TRACE("Forbidden error");
    throw Forbidden(res);
  }
};