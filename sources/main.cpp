#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <Server.hpp>
#include <Signal.hpp>
#include <Utility.hpp>

sig_atomic_t g_ExitStatus;

int main(int argc, char **argv) {
  if (argc > 2) {
    LOG_INFO(ERR_MSG_USAGE);
    return (int)Error::Args;
  } else if (argc == 2) {
    Config config(argv[1]);
  } else {
    Config config(void);
  }
  Signal::trackSignals();
  return g_ExitStatus;
}
