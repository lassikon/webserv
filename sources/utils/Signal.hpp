#pragma once

#include <Global.hpp>
#include <Logger.hpp>

#include <csignal>

class Signal {
public:
  Signal(void){};
  Signal(const Signal &other) = delete;
  Signal &operator=(const Signal &other) = delete;
  ~Signal(void){};

public:
  static void TrackSignals(void) noexcept;

private:
  static inline void signalHandler(int sigNum) noexcept;
};
