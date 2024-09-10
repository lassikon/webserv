#pragma once

#include <HttpException.hpp>
#include <Response.hpp>

class PayloadTooLarge : public HttpException {
 public:
  PayloadTooLarge(Response& res) : HttpException(res, 413, "Payload Too Large") {}
};