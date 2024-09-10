#pragma once

#include <IRequestHandler.hpp>
#include <Request.hpp>
#include <Response.hpp>

class DeleteHandler : public IRequestHandler {
 public:
  DeleteHandler() = default;
  virtual ~DeleteHandler() = default;
  void executeRequest(Request& req, Response& res) override;
};