#pragma once

class Client;

class IServeAction {
 public:
  
  virtual ~IServeAction() {}
  virtual void execute(Client& client) = 0;
};
