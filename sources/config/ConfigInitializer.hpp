#pragma once
#include <Config.hpp>
#include <IDirectiveSetter.hpp>
#include <RouteDirectiveSetter.hpp>
#include <ServerDirectiveSetter.hpp>
#include <vector>

class ConfigInitializer {
 public:
  static Config initializeConfig(int argc, char **argv);
};