#pragma once

#include <poll.h>

#include <Config.hpp>
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
  void initServerConfigs(Config& config);

 private:
  void serverLoop(PollManager& pollManager);
};
