#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Client.hpp>
#include <Logger.hpp>
#include <PollManager.hpp>
#include <algorithm>
#include <cstring>
#include <iostream>

#define PORT "3490"
#define BACKLOG 10
#define MAX_CLIENTS 100

class Client;
class PollManager;

class Server {
 private:
  std::string port;
  int sockFD;
  std::vector<Client> clients;

 public:
  Server(const std::string& port);
  ~Server(void);
  void run(void);

 private:
  void setup(void);
  void acceptConnection(PollManager& pollManager);
  void handleClient(PollManager& PollManager, int clientFD);
  void cleanup(void);
};