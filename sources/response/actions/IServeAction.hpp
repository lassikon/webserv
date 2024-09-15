#pragma once

#include <NetworkException.hpp>
#include <Response.hpp>

class IServeAction {
 public:
  virtual void execute(Response& res) = 0;

  virtual ~IServeAction() {}
};
