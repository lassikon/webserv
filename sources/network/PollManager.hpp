#pragma once

#include <sys/epoll.h>
#include <Utility.hpp>
#include <Logger.hpp>
#include <functional>
#include <algorithm>
#include <vector>
#include <map>
#include <functional>


class PollManager {

#define MAX_CLIENTS 1000
#define MAX_EVENTS 64
#define TIMEOUT 1000

 private:
  int epollFd;
  std::vector<struct epoll_event> epollEvents;
  std::map<int, std::function<void(int)>> interestFdsList;

 public:
  PollManager(void);
  ~PollManager(void);

  int epollWait(void);
  std::vector<struct epoll_event>& getEpollEvents(void);
  std::map<int, std::function<void(int)>>& getInterestFdsList(void);

 public:
  void addFd(int fd, uint32_t events, std::function<void(int)> cleanUp);
  void removeFd(int fd);
  bool fdExists(int fd);
  void modifyFd(int fd, uint32_t events);
};
