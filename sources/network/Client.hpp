#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Config.hpp>
#include <Logger.hpp>
#include <Request.hpp>
#include <Response.hpp>
#include <Utility.hpp>
#include <cstring>
#include <iostream>
#include <map>
#include <unordered_set>
#include <vector>

enum struct ClientState {
  READING_REQLINE,
  READING_HEADER,
  READING_BODY,
  READING_DONE
};

class Client {
 private:
  int fd;
  ServerConfig& serverConfig;
  ClientState state;
  Request req;
  Response res;

 public:
  Client(int socketFd, ServerConfig& serverConfig);
  ~Client(void);

  bool operator==(const Client& other) const;
  bool handlePollEvents(short revents);
  bool receiveData(void);
  void processRequest(std::istringstream& iBuf, int nbytes);
  void handleRequest(void);
  bool sendResponse(void);

  // getters and setters
  int getFd(void) const { return fd; }
  Request& getReq(void) { return req; }
  void setFd(int fd);

  ClientState getState(void) const { return state; }
  void setState(ClientState state) { this->state = state; }

 private:
  void cleanupClient(void);
};
