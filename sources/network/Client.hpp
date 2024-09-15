#pragma once

#include <CgiHandler.hpp>
#include <DeleteHandler.hpp>
#include <GetHandler.hpp>
#include <PostHandler.hpp>

#include <Config.hpp>
#include <Logger.hpp>
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
#include <memory>
#include <sstream>
#include <string>
#include <vector>

enum struct ClientState { READING_REQLINE, READING_HEADER, READING_BODY, READING_DONE, DONE };

class Client {
 private:
  ClientState state;
  bool isCgi = false;
  int fd;

 private:
  std::vector<std::shared_ptr<ServerConfig>>& serverConfigs;

 private:
  Request req;
  Response res;

 private:
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

  bool handlePollEvents(short revents, int readFd, int writeFd);

 public:
  int getFd(void) const { return fd; }

  Request& getReq(void) { return req; }

  Response& getRes(void) { return res; }

  void setState(ClientState state) { this->state = state; }

  ClientState getState(void) const { return state; }

  bool getIsCgi(void) { return isCgi; }

  void setClientFd(int fd);

  void closeClientFd(void) { fd = -1; }

 private:
  ServerConfig chooseServerConfig();
  bool receiveData(int readFd);
  void buildPath(void);
  void cleanupClient(void);

 private:
  void processCgiOutput(void);
  bool isCgiOutput(std::string buf);

 private:
  void parseRequest(std::istringstream& iBuf, int nbytes);
  void processRequest(void);
  bool handleRequest(int writeFd);

 private:
  bool sendResponse(int writeFd);
  void resetResponse(void);
};
