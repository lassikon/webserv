#pragma once

#include <Server.hpp>

class Client {
 private:
  int fd;

 public:
  Client(int socketFD);
  ~Client(void);
  bool operator==(const Client& other);
  bool receiveData(void);
  int getFD(void);
  void setFD(int fd);
};