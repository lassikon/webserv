#pragma once

#include <IStateHandler.hpp>
#include <Logger.hpp>

class Client;

class SendState : public IStateHandler {
 public:
  SendState() = default;
  virtual ~SendState() = default;
  void execute(Client& client) override;
};