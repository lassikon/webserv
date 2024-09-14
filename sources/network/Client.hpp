#pragma once

#include <Config.hpp>
#include <DeleteHandler.hpp>
#include <GetHandler.hpp>
#include <HttpException.hpp>
#include <Logger.hpp>
#include <PostHandler.hpp>
#include <Request.hpp>
#include <Response.hpp>
#include <RuntimeException.hpp>
#include <Utility.hpp>

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <map>
#include <unordered_set>
#include <vector>

enum struct ClientState { READING_REQLINE, READING_HEADER, READING_BODY, READING_DONE };

class Client {
 private:
  int fd;
  std::vector<std::shared_ptr<ServerConfig>>& serverConfigs;
  ClientState state;
  Request req;
  Response res;
  GetHandler getHandler;
  PostHandler postHandler;
  DeleteHandler deleteHandler;

 public:
  Client(int socketFd, std::vector<std::shared_ptr<ServerConfig>>& serverConfigs);
  ~Client(void);

  bool operator==(const Client& other) const;
  bool handlePollEvents(short revents);

  // getters and setters
  int getFd(void) const { return fd; }

  Request& getReq(void) { return req; }

  void setFd(int fd);

  ClientState getState(void) const { return state; }

  void setState(ClientState state) { this->state = state; }

 private:
  bool receiveData(void);
  void parseRequest(std::istringstream& iBuf, int nbytes);
  void processRequest(void);
  bool handleRequest(void);
  bool sendResponse(void);
  void cleanupClient(void);
  ServerConfig chooseServerConfig();
};
