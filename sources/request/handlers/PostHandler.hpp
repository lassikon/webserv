#pragma once

#include <IRequestHandler.hpp>
#include <Request.hpp>
#include <Response.hpp>

class PostHandler : public IRequestHandler {
 public:
  PostHandler() = default;
  virtual ~PostHandler() = default;
  void executeRequest(Request& req, Response& res) override;
};