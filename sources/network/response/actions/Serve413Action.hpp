#pragma once

#include <IServeAction.hpp>
#include <Response.hpp>
#include <Logger.hpp>
#include <Utility.hpp>
#include <filesystem>
#include <string>
#include <filesystem>
#include <vector>


class Serve413Action : public IServeAction {
 public:
  Serve413Action() = default;
  virtual ~Serve413Action() = default;
  void execute(Response& res) override;
};