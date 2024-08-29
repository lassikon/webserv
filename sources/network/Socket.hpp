#pragma once

#include <fcntl.h>
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
  int port;

 public:
  Socket();
  ~Socket(void);

  int getFd(void) const { return sockFd; }
  void setupSocket(int port);

 private:
  void cleanupSocket(void);
  void setNonBlocking(void);
};
