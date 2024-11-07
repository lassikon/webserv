#include <Signal.hpp>

sigmap_t Signal::sigmap;

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
  g_ExitStatus = (int)RuntimeError::Signal + sigNum;
  for (auto& cgiParam : g_CgiParams) {
    kill(cgiParam.pid, SIGKILL);
  }
}

void Signal::sigPipeHandler(int sigNum) noexcept {
  (void)sigNum;
  LOG_DEBUG("SIGPIPE received");
}

void Signal::sigChildHandler(int sigNum) noexcept {
  (void)sigNum;
  LOG_DEBUG("Child process exited");
}

void Signal::trackSignals(void) noexcept {
  signal(SIGINT, signalHandler);
  signal(SIGQUIT, signalHandler);
  signal(SIGSTOP, signalHandler);
  signal(SIGTERM, signalHandler);
  signal(SIGKILL, signalHandler);
  signal(SIGPIPE, sigPipeHandler);
  //signal(SIGCHLD, sigChildHandler);
  createSigMap();
}
