#pragma once

#include <IServeAction.hpp>
#include <HttpException.hpp>
#include <NotFound.hpp>
#include <Response.hpp>

class Serve404Action : public IServeAction {
 public:
  Serve404Action() = default;
  virtual ~Serve404Action() = default;
  inline void execute(Response& res) override {
    LOG_TRACE("Serving Not Found error");
    throw NotFound(res);
  }
};