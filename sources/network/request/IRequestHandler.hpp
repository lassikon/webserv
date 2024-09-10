#pragma once

#include <Request.hpp>
#include <Response.hpp>

class IRequestHandler {
 public:
  virtual void executeRequest(Request& req, Response& res) = 0;
  virtual ~IRequestHandler() {}
};