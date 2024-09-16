#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>

class Client;
class ServeFileAction : public IServeAction {
 public:
  ServeFileAction() = default;
  virtual ~ServeFileAction() = default;
  void execute(Client& client) override;
};
