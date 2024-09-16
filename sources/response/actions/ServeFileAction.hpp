#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>
#include <Client.hpp>

class ServeFileAction : public IServeAction {
 public:
  ServeFileAction() = default;
  virtual ~ServeFileAction() = default;
  void execute(Client& client) override;
};
