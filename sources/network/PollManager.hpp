#pragma once

#include <sys/epoll.h>

#include <Client.hpp>
#include <Logger.hpp>
#include <algorithm>
#include <vector>

class PollManager {

#define MAX_CLIENTS 64
#define MAX_EVENTS 64
#define TIMEOUT 20000

 private:
  int epollFd;
  std::vector<struct epoll_event> epollEvents;

 public:
  PollManager(void);
  ~PollManager(void);

  int epollWait(void);
  std::vector<struct epoll_event>& getEpollEvents(void);

 public:
  void addFd(int fd, uint32_t events);
  void removeFd(int fd);
  bool fdExists(int fd);
  void modifyFd(int fd, uint32_t events);
};
