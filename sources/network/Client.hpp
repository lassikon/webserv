#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Config.hpp>
#include <CgiHandler.hpp>
#include <DeleteHandler.hpp>
#include <GetHandler.hpp>
#include <HttpException.hpp>
#include <Logger.hpp>
#include <PostHandler.hpp>
#include <Request.hpp>
#include <Response.hpp>
#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>
#include <Utility.hpp>
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
  CgiHandler cgiHandler;

 private:
  std::shared_ptr<ProcessTree> root;

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
  void buildPath(void);
  void processRequest(void);
  bool handleRequest(void);
  bool sendResponse(void);
  void cleanupClient(void);
  ServerConfig chooseServerConfig();

 private:
  template <typename... Args> void clientError(Args&&... args) {
    THROW(Error::Client, std::forward<Args>(args)...);
  }
};
