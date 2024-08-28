#include <Signal.hpp>

#define ERR_MSG_SIGNAL "Server interrupted by signal:"

void Signal::signalHandler(int sigNum) noexcept {
  LOG_TRACE(ERR_MSG_SIGNAL, sigNum);
  g_SignalReceived = sigNum;
  g_ExitStatus = sigNum;
}

void Signal::TrackSignals(void) noexcept {
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);
}
