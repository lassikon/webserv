#include <Signal.hpp>

SIGMAP Signal::sigmap;

void Signal::createSigMap(void) noexcept {
  sigmap.insert(std::make_pair(SIGINT, "Crtl + C"));
  sigmap.insert(std::make_pair(SIGQUIT, "Crtl + \\"));
}

void Signal::signalHandler(int sigNum) noexcept {
  std::cout << std::endl;
  LOG_INFO(ERR_MSG_SIGNAL, sigNum, sigmap.at(sigNum));
  g_ExitStatus = (int)Error::Signal + sigNum;
}

void Signal::trackSignals(void) noexcept {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  createSigMap();
}
