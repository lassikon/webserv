#pragma once

#include <IServeAction.hpp>
#include <Response.hpp>
#include <Logger.hpp>
#include <Utility.hpp>
#include <string>
#include <algorithm>

class ServeIndexAction : public IServeAction {
 public:
  ServeIndexAction() = default;
  virtual ~ServeIndexAction() = default;
  void execute(Response& res) override;
};