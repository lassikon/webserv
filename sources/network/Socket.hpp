#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Client.hpp>
#include <PollManager.hpp>
#include <string>

#define BACKLOG 10

class Socket {
 private:
  int sockFd;
  std::string port;

 public:
  Socket();
  ~Socket(void);

  int getFd(void) const { return sockFd; }
  void setupSocket(std::string port);
  void cleanupSocket(void);
};
