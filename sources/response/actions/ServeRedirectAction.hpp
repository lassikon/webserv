#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>

class ServeRedirectAction : public IServeAction {
 public:
  ServeRedirectAction() = default;
  virtual ~ServeRedirectAction() = default;
  void execute(Response& res) override;
};
