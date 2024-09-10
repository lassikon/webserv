#pragma once

#include <IServeAction.hpp>
#include <HttpException.hpp>
#include <PayloadTooLarge.hpp>
#include <Response.hpp>
#include <Logger.hpp>

class Serve413Action : public IServeAction {
 public:
  Serve413Action() = default;
  virtual ~Serve413Action() = default;
  inline void execute(Response& res) override{
    LOG_TRACE("Payload Too Large");
    throw PayloadTooLarge(res);
  }
};