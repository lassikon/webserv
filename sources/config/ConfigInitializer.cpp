#include <ConfigInitializer.hpp>

Config ConfigInitializer::initializeConfig() {
  std::unique_ptr<IDirectiveSetter> serverDirective =
      std::make_unique<ServerDirectiveSetter>();
  std::unique_ptr<IDirectiveSetter> routeDirective =
      std::make_unique<RouteDirectiveSetter>();
  return Config(std::move(serverDirective), std::move(routeDirective));
}