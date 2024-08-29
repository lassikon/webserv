#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Client.hpp>
#include <Logger.hpp>
#include <PollManager.hpp>
#include <Socket.hpp>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>
#include <Config.hpp>

#define PORT "3490"

class Server {
 private:
  Socket socket;
  std::string port;
  std::vector<Client> clients;
  std::shared_ptr<Config> config;

 public:
  Server(std::string port, std::shared_ptr<Config> config);
  ~Server(void);

  void runServer(void);

 private:
  void acceptConnection(PollManager& pollManager);
  void handleClient(PollManager& pollManager, int clientFd, short revents);
};