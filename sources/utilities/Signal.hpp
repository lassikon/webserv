#pragma once

#include <CgiHandler.hpp>
#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <Typedef.hpp>

#include <csignal>

class Signal {
 private:
  static SIGMAP sigmap;

 private:
  static void signalHandler(int sigNum) noexcept;
  static inline void createSigMap(void) noexcept;

 public:
  static void trackSignals(void) noexcept;
};
