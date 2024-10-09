#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>

class Client;
class ServeFileAction : public IServeAction {
 private:
  bool isIndexOrDefaultFile(Client& client);

 public:
  ServeFileAction() = default;
  virtual ~ServeFileAction() = default;
  void execute(Client& client) override;
};
