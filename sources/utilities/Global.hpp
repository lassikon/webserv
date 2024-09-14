#pragma once

#include <chrono>
#include <csignal>
#include <vector>

extern sig_atomic_t g_ExitStatus;
extern std::vector<struct CgiParams> g_CgiParams;