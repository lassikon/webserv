#pragma once

#include <IRequestHandler.hpp>
#include <Logger.hpp>
#include <filesystem>
#include <string>

class Client;

class DeleteHandler : public IRequestHandler {
 public:
  DeleteHandler() = default;
  virtual ~DeleteHandler() = default;
  void executeRequest(Client& client) override;

 private:
  std::string sanitizePath(std::string path);
};