#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>

class Client;
class ServeDirectoryListingAction : public IServeAction {
 public:
  ServeDirectoryListingAction() = default;
  virtual ~ServeDirectoryListingAction() = default;
  void execute(Client& client) override;
  std::string makeDirectoryListing(std::string path);
};
