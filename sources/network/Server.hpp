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

#define PORT "3490"

class Server {
 private:
  Socket socket;
  std::string port;
  std::vector<Client> clients;

 public:
  Server(std::string port);
  ~Server(void);

  int getSocketFd(void) const { return socket.getFd(); }
  std::string getPort(void) const { return port; }

  void acceptConnection(PollManager& pollManager);
  void handleClient(PollManager& pollManager, int clientFd);
};