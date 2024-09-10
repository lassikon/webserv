#pragma once

#include <HttpException.hpp>
#include <IServeAction.hpp>
#include <Logger.hpp>
#include <MethodNotAllowed.hpp>
#include <Response.hpp>

class Serve405Action : public IServeAction {
 public:
  Serve405Action() = default;
  virtual ~Serve405Action() = default;

  inline void execute(Response& res) override {
    LOG_TRACE("Method Not Allowed");
    throw MethodNotAllowed(res);
  }
};