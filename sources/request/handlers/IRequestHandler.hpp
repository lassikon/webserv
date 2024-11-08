#pragma once

class Client;

class IRequestHandler {
 public:
  virtual void executeRequest(Client& client) = 0;
  virtual ~IRequestHandler() {}
};
