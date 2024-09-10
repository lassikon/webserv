#pragma once

#include <HttpException.hpp>
#include <Response.hpp>

class Forbidden : public HttpException {
 public:
  Forbidden(Response& res) : HttpException(res, 403, "Forbidden") {}
};