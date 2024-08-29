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
#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

class Server {
 private:
  Socket socket;
  int port;
  std::vector<std::shared_ptr<Client>> clients;

 public:
  Server(ServerConfig& serverConfig);
  ~Server(void);

  int getSocketFd(void) const { return socket.getFd(); }
  int getPort(void) const { return port; }

  void acceptConnection(PollManager& pollManager);
  void handleClient(PollManager& pollManager, int clientFd);
  bool isClientFd(int fd) const;
  void handleResponse(PollManager& pollManager, int clientFd);
};