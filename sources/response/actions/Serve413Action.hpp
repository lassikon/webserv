#pragma once

#include <IServeAction.hpp>
class Client;
class Serve413Action : public IServeAction {
 public:
  Serve413Action() = default;
  virtual ~Serve413Action() = default;
  void execute(Client& client) override;
};
