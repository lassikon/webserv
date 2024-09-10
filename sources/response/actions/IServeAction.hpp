#pragma once
#include <Response.hpp>

class IServeAction {
 public:
  virtual void execute(Response& res) = 0;
  virtual ~IServeAction() {}
};