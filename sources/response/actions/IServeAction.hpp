#pragma once

#include <Client.hpp>

class IServeAction {
 public:
  virtual void execute(Client& client) = 0;

  virtual ~IServeAction() {}
};
