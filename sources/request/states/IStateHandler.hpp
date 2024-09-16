#pragma once
#include "Client.hpp"

class IStateHandler {
 public:
  virtual void execute(Client& client) = 0;
  virtual ~IStateHandler() {}
};