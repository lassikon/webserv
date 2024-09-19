#pragma once

#include <CgiHandler.hpp>
#include <DeleteHandler.hpp>
#include <GetHandler.hpp>
#include <PostHandler.hpp>

#include <ParseState.hpp>
#include <ProcessState.hpp>
#include <ReadState.hpp>
#include <SendState.hpp>

#include <Config.hpp>
#include <Logger.hpp>
#include <Request.hpp>
#include <Response.hpp>
#include <Utility.hpp>

#include <RuntimeException.hpp>

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

enum struct ClientState { READING, PROCESSING, PREPARING ,SENDING, DONE };
enum struct ParsingState { REQLINE, HEADER, BODY, DONE };
enum struct CgiState { READING, WRITING, DONE};

class Client {
 private:
  Request req;
  Response res;
  int fd;

 private:
  std::vector<std::shared_ptr<ServerConfig>>& serverConfigs;

 private:  // handlers
  PostHandler postHandler;
  DeleteHandler deleteHandler;
  CgiHandler cgiHandler;
  GetHandler getHandler;

 private:  //states
  ClientState clientState;
  ParsingState parsingState;
  CgiState cgiState;
  ReadState readState;
  ParseState parseState;
  ProcessState processState;
  SendState sendState;

 private:  //read
  int readFd;
  std::shared_ptr<std::vector<char>> readBuf = nullptr;
  ssize_t readNBytes = 0;

 public:  //read
  int getReadFd(void) { return readFd; }
  std::shared_ptr<std::vector<char>> getReadBuf(void) { return readBuf; }
  ssize_t getReadNBytes(void) { return readNBytes; }
  void setReadFd(int fd) { readFd = fd; }
  void setReadNBytes(ssize_t nBytes) { readNBytes = nBytes; }
  void setReadBuf(std::shared_ptr<std::vector<char>> buf) { readBuf = buf; }

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

 private:
  void handlePollInEvent(int readFd);
  void handlePollOutEvent(int writeFd);
  bool shouldCloseConnection(void);
  void cleanupClient(void);

 public:  // getters
  int getFd(void) const { return fd; }
  Request& getReq(void) { return req; }
  Response& getRes(void) { return res; }
  PostHandler& getPostHandler(void) { return postHandler; }
  DeleteHandler& getDeleteHandler(void) { return deleteHandler; }
  CgiHandler& getCgiHandler(void) { return cgiHandler; }
  GetHandler& getGetHandler(void) { return getHandler; }
  std::vector<std::shared_ptr<ServerConfig>>& getServerConfigs(void) { return serverConfigs; }
  ClientState getClientState(void) const { return clientState; }
  ParsingState getParsingState(void) const { return parsingState; }
  CgiState getCgiState(void) const { return cgiState; }

 public:  //  setters
  void setFd(int fd);
  void setClientState(ClientState state) { clientState = state; }
  void setParsingState(ParsingState state) { parsingState = state; }
  void setCgiState(CgiState state) { cgiState = state; }
};
