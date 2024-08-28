#pragma once

#include <Server.hpp>
#include <vector>

class ServersManager {
 private:
  std::vector<Server> servers;

 public:
  ServersManager(void);
  ~ServersManager(void);

  void runServers(void);
};
