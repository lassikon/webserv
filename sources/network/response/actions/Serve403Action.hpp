#pragma once

#include <IServeAction.hpp>
#include <Response.hpp>
#include <Logger.hpp>
#include <Utility.hpp>
#include <filesystem>
#include <string>
#include <filesystem>
#include <vector>

class Serve403Action : public IServeAction {
 public:
  Serve403Action() = default;
  virtual ~Serve403Action() = default;
  void execute(Response& res) override;
};