#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>
#include <Response.hpp>
#include <string>
#include <algorithm>


class ServeDefaultFileAction : public IServeAction {
 public:
  ServeDefaultFileAction() = default;
  virtual ~ServeDefaultFileAction() = default;
  void execute(Response& res) override;
};