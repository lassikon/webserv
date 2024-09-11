#pragma once

#include <HttpException.hpp>
#include <Response.hpp>

class BadRequest : public HttpException {
 public:
  BadRequest(Response& res) : HttpException(res, 400, "Bad Request") {}
};