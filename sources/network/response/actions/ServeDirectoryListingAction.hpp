#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>
#include <Response.hpp>
#include <string>

class ServeDirectoryListingAction : public IServeAction {
 public:
  ServeDirectoryListingAction() = default;
  virtual ~ServeDirectoryListingAction() = default;
  void execute(Response& res) override;

};