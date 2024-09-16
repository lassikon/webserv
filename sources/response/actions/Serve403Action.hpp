#pragma once

#include <IServeAction.hpp>

class Client;
class Serve403Action : public IServeAction {
 public:
  Serve403Action() = default;
  virtual ~Serve403Action() = default;
  void execute(Client& client) override;
};
