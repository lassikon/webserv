#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>
#include <UrlEncoder.hpp>

class Client;

class ServeQueryAction : public IServeAction {
 public:
  ServeQueryAction() = default;
  virtual ~ServeQueryAction() = default;
  void execute(Client& client) override;
  void serveWrongQuery(Client& client);
};
