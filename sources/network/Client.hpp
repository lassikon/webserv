#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <CgiHandler.hpp>
#include <Config.hpp>
#include <DeleteHandler.hpp>
#include <Exception.hpp>
#include <GetHandler.hpp>
#include <HttpException.hpp>
#include <IRequestHandler.hpp>
#include <Logger.hpp>
#include <PostHandler.hpp>
#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>
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
  READING_DONE,
  DONE
};

class Client {
 private:
  int fd;
  bool isCgi = false;
  std::vector<std::shared_ptr<ServerConfig>>& serverConfigs;
  ClientState state;
  Request req;
  Response res;
  PostHandler postHandler;
  DeleteHandler deleteHandler;
  CgiHandler cgiHandler;
  GetHandler getHandler;

 private:
  std::shared_ptr<ProcessTree> root;

 public:
  Client(int socketFd, std::vector<std::shared_ptr<ServerConfig>>& serverConfigs);
  ~Client(void);

  bool operator==(const Client& other) const;
  bool handlePollEvents(short revents, int readFd, int writeFd);

  // getters and setters
  int getFd(void) const { return fd; }

  Request& getReq(void) { return req; }

  Response& getRes(void) { return res; }
  bool getIsCgi(void) { return isCgi; }

  void setFd(int fd);

  ClientState getState(void) const { return state; }

  void setState(ClientState state) { this->state = state; }

 private:
  bool isCgiOutput(std::string buf);
  bool receiveData(int readFd);
  void parseRequest(std::istringstream& iBuf, int nbytes);
  void buildPath(void);
  void processRequest(void);
  void processCgiOutput(void);
  bool handleRequest(int writeFd);
  bool sendResponse(int writeFd);
  void cleanupClient(void);
  ServerConfig chooseServerConfig();
  void resetResponse(void);

 private:
  template <typename... Args> void clientError(Args&&... args) {
    THROW(Error::Client, std::forward<Args>(args)...);
  }
};
