#pragma once

#include <Server.hpp>
#include <map>
#include <Utility.hpp>

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

 public:
  Client(int socketFd);
  ~Client(void);
  bool operator==(const Client& other) const;
  bool receiveData(void);
  int getFd(void) const;
  void setFd(int fd);

  // request
  void processRequest(std::string& buf);
  void parseRequestLine(HttpReq& req, std::string& requestLine);
  void parseHeaders(HttpReq& req, std::istringstream& iBuf);
  void parseBody(HttpReq& req, std::istringstream& iBuf);
  void handleRequest(HttpReq& req);
  void sendResponse(HttpReq& req);
};