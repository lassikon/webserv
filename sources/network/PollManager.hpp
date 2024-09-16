#pragma once

#include <poll.h>

#include <Client.hpp>
#include <Logger.hpp>
#include <algorithm>
#include <vector>

class PollManager {

#define MAX_CLIENTS 100
#define TIMEOUT 5000

 private:
  std::vector<struct pollfd> pollFds;

 public:
  PollManager(void);
  ~PollManager(void);

  int pollFdsCount(void);
  std::vector<struct pollfd>& getPollFds(void);

 public:
  void addFd(int fd, short events);
  void removeFd(int fd);
  bool fdExists(int fd);
};
