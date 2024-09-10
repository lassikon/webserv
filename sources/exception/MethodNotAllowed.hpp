#pragma once

#include <HttpException.hpp>
#include <Response.hpp>

class MethodNotAllowed : public HttpException {
 public:
  MethodNotAllowed(Response& res) : HttpException(res, 405, "Method Not Allowed") {}
};