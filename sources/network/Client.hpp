#pragma once

#include <Server.hpp>

class Client {
 private:
  int fd;

 public:
  Client(int socketFd);
  ~Client(void);
  bool operator==(const Client& other);
  bool receiveData(void);
  int getFd(void);
  void setFd(int fd);
};