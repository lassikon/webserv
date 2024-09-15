#pragma once

#include <Logger.hpp>
#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>
#include <RuntimeException.hpp>

class Client;

class IRequestHandler {
 public:
  virtual void executeRequest(Client& client) = 0;

  virtual ~IRequestHandler() {}
};
