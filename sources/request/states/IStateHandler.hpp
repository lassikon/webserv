#pragma once

class Client;
class IStateHandler {
 public:
  virtual void execute(Client& client) = 0;
  virtual ~IStateHandler() {}
};