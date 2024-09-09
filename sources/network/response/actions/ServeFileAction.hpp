#pragma once

#include <IServeAction.hpp>
#include <Response.hpp>
#include <Logger.hpp>
#include <string>

class ServeFileAction : public IServeAction {
 public:
  ServeFileAction() = default;
  virtual ~ServeFileAction() = default;
  void execute(Response& res) override;
};