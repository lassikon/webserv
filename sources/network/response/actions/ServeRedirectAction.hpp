#pragma once

#include <IServeAction.hpp>
#include <Response.hpp>
#include <Logger.hpp>
#include <string>

class ServeRedirectAction : public IServeAction {
 public:
  ServeRedirectAction() = default;
  virtual ~ServeRedirectAction() = default;
  void execute(Response& res) override;
};