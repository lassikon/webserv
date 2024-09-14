#pragma once

#include <Client.hpp>
#include <Config.hpp>
#include <Exception.hpp>
#include <Logger.hpp>
#include <PollManager.hpp>
#include <Socket.hpp>

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

class Server {
 private:
  Socket socket;
  int port;
  std::string ipAddress;
  std::string serverName;
  std::vector<std::shared_ptr<Client>> clients;
  // ServerConfig& serverConfig;
  std::vector<std::shared_ptr<ServerConfig>> serverConfigs;

 public:
  Server(ServerConfig& serverConfig);
  ~Server(void);

  int getSocketFd(void) const { return socket.getFd(); }

  int getPort(void) const { return port; }

  std::string getServerName(void) const { return serverName; }

  std::string getIpAddress(void) const { return ipAddress; }

  void addServerConfig(ServerConfig& serverConfig);
  void acceptConnection(PollManager& pollManager);
  void handleClient(PollManager& pollManager, int clientFd, short revents);
  bool isClientFd(int fd) const;
};
