#pragma once

#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>

#include <csignal>
#include <string>
#include <unordered_map>

class Signal {
private:
  std::unordered_map<int, std::string> signals;

public:
  Signal(void);

private:
  static void signalHandler(int sigNum) noexcept;
  static inline std::string sigNumToString(int sigNum);

public:
  static void trackSignals(void) noexcept;
};
