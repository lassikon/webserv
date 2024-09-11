#pragma once

#include <IServeAction.hpp>
#include <Response.hpp>
#include <Logger.hpp>
#include <Utility.hpp>
#include <filesystem>
#include <string>
#include <filesystem>
#include <vector>


class Serve405Action : public IServeAction {
 public:
  Serve405Action() = default;
  virtual ~Serve405Action() = default;
  void execute(Response& res) override;
};