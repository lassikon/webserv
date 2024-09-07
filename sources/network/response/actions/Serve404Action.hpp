#pragma once

#include <IServeAction.hpp>
#include <Response.hpp>
#include <Logger.hpp>
#include <Utility.hpp>
#include <filesystem>
#include <string>
#include <filesystem>
#include <vector>
#include <algorithm>

class Serve404Action : public IServeAction {
 public:
  Serve404Action() = default;
  virtual ~Serve404Action() = default;
  void execute(Response& res) override;
};