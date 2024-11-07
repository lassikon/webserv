#pragma once

#include <chrono>
#include <csignal>
#include <vector>
#include <atomic>

extern sig_atomic_t g_ExitStatus;
extern std::atomic<int> g_timeOut;
extern std::vector<struct CgiParams> g_CgiParams;