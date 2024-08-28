#pragma once

#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <vector>


#include <map>
#include <Utility.hpp>
#include <Logger.hpp>

enum struct ClientState { READING_REQLINE, READING_HEADER, READING_BODY, READING_DONE };

struct HttpReq {
  std::string method;
  std::string path;
  std::string version;
  std::map<std::string, std::string> headers;
  std::string body;
  bool transferEncodingChunked;
};

class Client {
 private:
  int fd;
  HttpReq req;
  ClientState state;

 public:
  Client(int socketFd);
  ~Client(void);

  bool operator==(const Client& other) const;
  bool receiveData(void);
  int getFd(void) const { return fd; }
  void setFd(int fd);

  // request
  void processRequest(std::istringstream& iBuf);
  void parseRequestLine(HttpReq& req, std::string& requestLine);
  void parseHeaders(HttpReq& req, std::istringstream& iBuf);
  void parseBody(HttpReq& req, std::istringstream& iBuf);
  void handleRequest(HttpReq& req);

  // response
  bool sendResponse(void);
	private:
	void cleanupClient(void);
};
