#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>
#include <Response.hpp>
#include <string>
#include <algorithm>
#include <filesystem>
#include <sstream>

class ServeDirectoryListingAction : public IServeAction {
 public:
  ServeDirectoryListingAction() = default;
  virtual ~ServeDirectoryListingAction() = default;
  void execute(Response& res) override;
  std::string makeDirectoryListing(std::string path);

};