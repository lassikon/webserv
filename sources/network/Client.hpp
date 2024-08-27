#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Logger.hpp>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>

class Client {
 private:
  int fd;

 public:
  Client(int socketFd);
  ~Client(void);

  bool operator==(const Client& other) const;
  bool receiveData(void);
  int getFd(void) const { return fd; }
  void setFd(int fd);

	private:
	void cleanupClient(void);
};