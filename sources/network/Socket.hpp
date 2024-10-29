#pragma once

#include <Client.hpp>
#include <Logger.hpp>
#include <PollManager.hpp>
#include <RuntimeException.hpp>
#include <Utility.hpp>

#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

class Socket {

#define BACKLOG 10

 private:
  int sockFd = -1;

 public:
  Socket(void);
  ~Socket(void);

  int getFd(void) const { return sockFd; }
  void setupSocket(ServerConfig& serverConfig);

 private:
  void closeSockedFd(void) { sockFd = 0; }
  void cleanupSocket(void);
  void setNonBlocking(void);
};
