#pragma once

#include <IRequestHandler.hpp>
#include <Request.hpp>
#include <Response.hpp>

class ProcessTree;
class ProcessTreeBuilder;

class GetHandler : public IRequestHandler {
 public:
  GetHandler() = default;
  virtual ~GetHandler() = default;
  void executeRequest(Request& req, Response& res) override;

 private:
  std::shared_ptr<ProcessTree> root;
};