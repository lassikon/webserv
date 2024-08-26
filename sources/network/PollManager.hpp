#pragma once

#include <Server.hpp>
#include <vector>

#define TIMEOUT 10000

class PollManager {
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