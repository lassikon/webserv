#pragma once

#include <Server.hpp>

class Client {
 private:
 public:
  int fd;
  Client(int socketFD);
  ~Client(void);
  bool operator==(const Client& other);
  bool receiveData(void);
  int getFD(void);
  void setFD(int fd);
};