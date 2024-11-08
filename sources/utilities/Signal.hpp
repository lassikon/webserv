#pragma once

#include <CgiHandler.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <Typedef.hpp>

#include <csignal>

class Signal {
 private:
  static sigmap_t sigmap;

 private:
  static void signalHandler(int sigNum) noexcept;
  static inline void createSigMap(void) noexcept;
  static void sigPipeHandler(int sigNum) noexcept;
  static void sigChildHandler(int sigNum) noexcept;

 public:
  static void trackSignals(void) noexcept;
};
