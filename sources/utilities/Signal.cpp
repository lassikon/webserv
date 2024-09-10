#include <Signal.hpp>

SIGMAP Signal::sigmap;

void Signal::createSigMap(void) noexcept {
  sigmap.insert(std::make_pair(SIGINT, "Crtl + C"));
  sigmap.insert(std::make_pair(SIGQUIT, "Crtl + \\"));
  sigmap.insert(std::make_pair(SIGSTOP, "Crtl + Z"));
  sigmap.insert(std::make_pair(SIGTERM, "Terminate"));
  sigmap.insert(std::make_pair(SIGKILL, "Kill"));
}

void Signal::signalHandler(int sigNum) noexcept {
  std::cout << std::endl;
  LOG_INFO("Server interrupted by signal:", sigNum, sigmap.at(sigNum));
  g_ExitStatus = (int)Error::Signal + sigNum;
  CgiHandler::killAllChildPids();
}

void Signal::trackSignals(void) noexcept {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  signal(SIGSTOP, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGKILL, signalHandler);
  createSigMap();
}
