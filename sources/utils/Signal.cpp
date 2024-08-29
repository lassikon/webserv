#include <Signal.hpp>

void Signal::signalHandler(int sigNum) noexcept {
  LOG_INFO(ERR_MSG_SIGNAL, sigNum);
  g_ExitStatus = (int)Error::Signal + sigNum;
}

void Signal::TrackSignals(void) noexcept {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
}
