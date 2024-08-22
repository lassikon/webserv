#pragma once

#include <Server.hpp>

class PollManager {
 private:
  std::vector<struct pollfd> pollFDs;

 public:
  PollManager(void);
  ~PollManager(void);
  void addFD(int fd, short events);
  void removeFD(int fd);
  int pollFDsCount(void);
  std::vector<struct pollfd>& getPollFDs(void);
};