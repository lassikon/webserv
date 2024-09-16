#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Client.hpp>
#include <Config.hpp>
#include <Logger.hpp>
#include <PollManager.hpp>
#include <Socket.hpp>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <vector>

class Server {
 private:
  Socket socket;
  int port;
  std::string ipAddress;
  std::string serverName;
  std::vector<std::shared_ptr<Client>> clients;
  std::vector<std::shared_ptr<ServerConfig>> serverConfigs;
  std::map<int, std::chrono::steady_clock::time_point> clientLastActivity;
  const std::chrono::seconds idleTimeout = std::chrono::seconds(10);

 public:
  Server(ServerConfig& serverConfig);
  ~Server(void);

  int getSocketFd(void) const { return socket.getFd(); }

  int getPort(void) const { return port; }

  std::string getServerName(void) const { return serverName; }

  std::string getIpAddress(void) const { return ipAddress; }

  void addServerConfig(ServerConfig& serverConfig);
  void acceptConnection(PollManager& pollManager);
  void handleClient(PollManager& pollManager, short revents, int readFd, int clientFd);
  void checkIdleClients(PollManager& pollManager);
  void updateClientLastActivity(int clientFd);
  bool isClientFd(int fd) const;
  
};