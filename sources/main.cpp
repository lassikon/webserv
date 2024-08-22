#include <Logger.hpp>

int main() {
  LOG_TRACE("Hello ", 123);
  LOG_DEBUG("Hello ", 123);
  LOG_INFO("Hello ", 123);
  LOG_WARN("Hello ", 123);
  LOG_ERROR("Hello ", 123);
  LOG_FATAL("Hello ", 123);
  return 0;
}
