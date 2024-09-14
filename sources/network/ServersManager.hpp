#pragma once

#include <Config.hpp>
#include <PollManager.hpp>
#include <Server.hpp>

#include <poll.h>

#include <memory>
#include <vector>

class ServersManager {
 private:
  std::vector<std::shared_ptr<Server>> servers;
  std::vector<struct pollfd> pollFds;

 public:
  ServersManager(void);
  ~ServersManager(void);

  void runServers(void);
  void configServers(Config& config);

 private:
  void serverLoop(PollManager& pollManager);
  bool checkServerExists(ServerConfig& serverConfig);
};
