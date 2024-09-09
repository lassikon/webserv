#pragma once

#include <poll.h>

#include <Config.hpp>
#include <PollManager.hpp>
#include <Server.hpp>
#include <string>
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

  template <typename... Args> void serverError(Args&&... args) {
    THROW(Error::Server, std::forward<Args>(args)..., STRERROR);
  }
};
