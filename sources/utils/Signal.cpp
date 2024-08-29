#include <Signal.hpp>
#include <csignal>
#include <utility>

Signal::Signal(void) {
  signals.insert(std::make_pair(SIGINT, "Crtl + C"));
  signals.insert(std::make_pair(SIGQUIT, "Crtl + \\"));
}

std::string Signal::sigNumToString(int sigNum) {
  (void)sigNum;
  return "Crtl + ?";
}

void Signal::signalHandler(int sigNum) noexcept {
  LOG_INFO(ERR_MSG_SIGNAL, sigNum);
  g_ExitStatus = (int)Error::Signal + sigNum;
}

void Signal::trackSignals(void) noexcept {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
}
