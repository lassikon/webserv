#pragma once

#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Client.hpp>
#include <Config.hpp>
#include <IException.hpp>
#include <Logger.hpp>
#include <NetworkException.hpp>
#include <PollManager.hpp>
#include <SessionManager.hpp>
#include <Socket.hpp>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Server {
 private:
  std::vector<std::shared_ptr<Client>> clients;
  std::vector<std::shared_ptr<ServerConfig>> serverConfigs;
  std::map<int, steady_time_point_t> clientLastActivity;

 private:
  const std::chrono::seconds idleTimeout = std::chrono::seconds(g_timeOut);

 private:
  std::string ipAddress;
  std::string serverName;
  SessionManager session;
  Socket socket;
  int port;

 public:
  Server(ServerConfig& serverConfig);
  ~Server(void);

  void addServerConfig(ServerConfig& serverConfig);
  void acceptConnection(PollManager& pollManager);
  void handleClientIn(PollManager& pollManager, uint32_t revents, int eventFd,
                      int clientFd);
  void handleClientOut(PollManager& pollManager, uint32_t revents, int eventFd,
                       int clientFd);
  void checkIdleClients(PollManager& pollManager);
  void updateClientLastActivity(int clientFd);
  void modifyFdEvent(PollManager& pollManager, std::shared_ptr<Client> client,
                     int eventFd, int clientFd);

 public:
  int getSocketFd(void) const { return socket.getFd(); }
  int getPort(void) const { return port; }
  void removeClient(PollManager& pollManager, int clientFd);
  std::string getServerName(void) const { return serverName; }
  SessionManager& getSession(void) { return session; }
  std::vector<std::shared_ptr<Client>>& getClients(void) { return clients; }
  std::vector<std::shared_ptr<ServerConfig>>& getServerConfigs(void) { return serverConfigs;}
  std::string getIpAddress(void) const { return ipAddress; }
  bool isClientFd(int fd) const;
};
