#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>

class ServeFileAction : public IServeAction {
 public:
  ServeFileAction() = default;
  virtual ~ServeFileAction() = default;
  void execute(Response& res) override;
};
