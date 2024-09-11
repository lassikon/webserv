#pragma once

#include <HttpException.hpp>
#include <Response.hpp>

class NotFound : public HttpException {
 public:
  NotFound(Response& res) : HttpException(res, 404, "Not Found") {}
};