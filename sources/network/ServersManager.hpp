#pragma once

#include <poll.h>

#include <PollManager.hpp>
#include <Server.hpp>
#include <string>
#include <vector>

class ServersManager {
 private:
  std::vector<Server> servers;
  std::vector<struct pollfd> pollFds;

 public:
  ServersManager(void);
  ~ServersManager(void);

  void runServers(void);

 private:
  void serverLoop(PollManager& pollManager);
};
