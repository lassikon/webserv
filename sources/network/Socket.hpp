#pragma once

#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Client.hpp>
#include <PollManager.hpp>
#include <RuntimeException.hpp>

#define BACKLOG 10

class Socket {
 private:
  int sockFd;
  int port;

 public:
  Socket(void);
  ~Socket(void);

  int getFd(void) const { return sockFd; }

  void setupSocket(ServerConfig& serverConfig);

 private:
  void setSockFdClosed(void) { sockFd = 0; }

  void cleanupSocket(void);
  void setNonBlocking(void);
};
