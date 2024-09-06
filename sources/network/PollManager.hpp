#pragma once

// #include <Server.hpp>
#include <poll.h>

#include <Client.hpp>
#include <Logger.hpp>
#include <algorithm>
#include <vector>

class PollManager {
#define MAX_CLIENTS 100
#define TIMEOUT -1

 private:
  std::vector<struct pollfd> pollFds;

 public:
  PollManager(void);
  ~PollManager(void);

  void addFd(int fd, short events);
  void removeFd(int fd);
  int pollFdsCount(void);
  std::vector<struct pollfd>& getPollFds(void);
};