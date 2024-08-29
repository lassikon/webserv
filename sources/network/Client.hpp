#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <map>
#include <unordered_set>
#include <vector>

#include <Config.hpp>
#include <Logger.hpp>
#include <Utility.hpp>
#include <Request.hpp>
#include <Response.hpp>
#include <ResourceManager.hpp>


enum struct ClientState {
  READING_REQLINE,
  READING_HEADER,
  READING_BODY,
  READING_DONE
};

class Client {
 private:
  int fd;
  ClientState state;
  Request req;
  Response res;
  ResourceManager resourceManager;
  std::shared_ptr<Config> config;

 public:
  Client(int socketFd);
  ~Client(void);

  bool operator==(const Client& other) const;
  bool handlePollEvents(short revents);
  bool receiveData(void);
  void processRequest(std::istringstream& iBuf);
  void handleRequest(void);
  bool sendResponse(void);

  //getters and setters
  int getFd(void) const { return fd; }
  void setFd(int fd);

  ClientState getState(void) const { return state; }
  void setState(ClientState state) { this->state = state; }
  void setConfig(std::shared_ptr<Config> config) { this->config = config; }

 private:
  void cleanupClient(void);
};
