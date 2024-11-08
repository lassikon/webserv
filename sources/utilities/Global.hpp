#pragma once

#include <atomic>
#include <csignal>
#include <vector>

extern sig_atomic_t g_ExitStatus;
extern std::atomic<int> g_timeOut;
extern std::vector<struct CgiParams> g_CgiParams;
