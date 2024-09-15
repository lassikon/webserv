#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Config.hpp>
#include <Exception.hpp>
#include <HttpException.hpp>
#include <Logger.hpp>
#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>
#include <Utility.hpp>

#include <Request.hpp>
#include <Response.hpp>

#include <CgiHandler.hpp>
#include <DeleteHandler.hpp>
#include <GetHandler.hpp>
#include <IRequestHandler.hpp>
#include <PostHandler.hpp>

#include <IStateHandler.hpp>
#include <ParseState.hpp>
#include <ProcessState.hpp>
#include <ReadState.hpp>
#include <SendState.hpp>

#include <cstring>
#include <iostream>
#include <map>
#include <unordered_set>
#include <vector>

enum struct ClientState { READING, PROCESSING, SENDING, DONE };
enum struct ParsingState { REQLINE, HEADER, BODY, DONE };

class Client {
 private:
  int fd;
  bool isCgi = false;
  std::vector<std::shared_ptr<ServerConfig>>& serverConfigs;
  Request req;
  Response res;

 private:  // handlers
  std::shared_ptr<ProcessTree> root;
  PostHandler postHandler;
  DeleteHandler deleteHandler;
  CgiHandler cgiHandler;
  GetHandler getHandler;

 private:  //states
  ClientState clientState;
  ParsingState parsingState;
  ReadState readState;
  ParseState parseState;
  ProcessState processState;
  SendState sendState;

 private:  //read
  int readFd;
  char* readBuf = nullptr;
  ssize_t readNBytes = 0;

 public:  //read
  int getReadFd(void) { return readFd; }
  char* getReadBuf(void) { return readBuf; }
  ssize_t getReadNBytes(void) { return readNBytes; }
  void setReadFd(int fd) { readFd = fd; }
  void setReadNBytes(ssize_t nBytes) { readNBytes = nBytes; }
  void setReadBuf(char* buf) { readBuf = buf; }

 private:  //write
  int writeFd;
  ssize_t writeNBytes = 0;

 public:  //write
  int getWriteFd(void) { return writeFd; }
  ssize_t getWriteNBytes(void) { return writeNBytes; }
  void setWriteFd(int fd) { writeFd = fd; }
  void setWriteNBytes(ssize_t nBytes) { writeNBytes = nBytes; }

 public:
  Client(int socketFd, std::vector<std::shared_ptr<ServerConfig>>& serverConfigs);
  ~Client(void);

  bool operator==(const Client& other) const;

 public:
  void resetRequest(void);
  void resetResponse(void);
  void initClient(void);
  bool handlePollEvents(short revents, int readFd, int writeFd);

 public:  // getters and setters
  int getFd(void) const { return fd; }
  bool getIsCgi(void) { return isCgi; }
  Request& getReq(void) { return req; }
  Response& getRes(void) { return res; }
  PostHandler& getPostHandler(void) { return postHandler; }
  DeleteHandler& getDeleteHandler(void) { return deleteHandler; }
  CgiHandler& getCgiHandler(void) { return cgiHandler; }
  GetHandler& getGetHandler(void) { return getHandler; }
  std::vector<std::shared_ptr<ServerConfig>>& getServerConfigs(void) { return serverConfigs; }
  ClientState getClientState(void) const { return clientState; }
  ParsingState getParsingState(void) const { return parsingState; }
  void setFd(int fd);
  void setClientState(ClientState state) { clientState = state; }
  void setParsingState(ParsingState state) { parsingState = state; }
  void setIsCgi(bool isCgi) { this->isCgi = isCgi; }

 private:
  void handlePollInEvent(int readFd);
  void handlePollOutEvent(int writeFd);
  bool shouldCloseConnection(void);
  void cleanupClient(void);

 private:
  template <typename... Args> void clientError(Args&&... args) {
    THROW(Error::Client, std::forward<Args>(args)...);
  }
};
