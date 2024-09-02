#pragma once

#include <Client.hpp>
#include <Exception.hpp>
#include <PollManager.hpp>

#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#define BACKLOG 10

class Socket {
private:
  int sockFd;
  int port;

public:
  Socket(void);
  ~Socket(void);

  int getFd(void) const { return sockFd; }
  void setupSocket(int port);

private:
  void cleanupSocket(void);
  void setNonBlocking(void);

  template <typename... Args> void socketError(Args &&...args) {
    cleanupSocket();
    THROW(Error::Socket, std::forward<Args>(args)..., STRERROR);
  }
};
