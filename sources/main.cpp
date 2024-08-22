#include <Logger.hpp>

int main(void) {
  LOG_TRACE("Hello World! ", "This number is: ", 666);
  LOG_DEBUG("Hello World! ", "This number is: ", 666);
  LOG_INFO("Server is running...");
  return 0;
}
