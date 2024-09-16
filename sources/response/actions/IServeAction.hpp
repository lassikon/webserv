#pragma once

#include <NetworkException.hpp>

class IServeAction {
 public:
  virtual void execute(Client& client) = 0;

  virtual ~IServeAction() {}
};
