#include <ConfigInitializer.hpp>

Config ConfigInitializer::initializeConfig(int argc, char **argv) {
  std::unique_ptr<IDirectiveSetter> serverDirective =
      std::make_unique<ServerDirectiveSetter>();
  std::unique_ptr<IDirectiveSetter> routeDirective =
      std::make_unique<RouteDirectiveSetter>();
  std::string configFilePath = "confDefault/default.conf";
  if (argc == 2) {
    configFilePath = std::string(argv[1]);
  }
  return Config(std::move(serverDirective), std::move(routeDirective), configFilePath);
}