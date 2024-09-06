#pragma once

#include <map>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <memory>

#include <Utility.hpp>
#include <Logger.hpp>

class Client;

class Request {
 private:
  std::string reqMethod;
  std::string reqURI;
  std::string reqVersion;
  std::map<std::string, std::string> reqHeaders;
  std::string reqBody;
  size_t reqBodySize = 0;
  bool transferEncodingChunked;

 public:
  Request(void) {transferEncodingChunked = false;}
  ~Request(void) {}

  std::map<std::string, std::string>& getHeaders(void) { return reqHeaders; }

  void parseRequestLine(Client* client, std::string& requestLine);
  void parseHeaders(Client* client, std::istringstream& iBuf);
  void parseBody(Client* client, std::istringstream& , int nbytes);
  void parseChunkedBody(Client* client, std::istringstream& iBuf);

  std::string getMethod(void) const { return reqMethod; }
  std::string getReqURI(void) const { return reqURI; }
  std::string getVersion(void) const { return reqVersion; }
  size_t getBodySize(void) const { return reqBodySize; }
  std::map<std::string, std::string> getHeaders(void) const { return reqHeaders; }
  std::string getBody(void) const { return reqBody; }
  bool isTransferEncodingChunked(void) const { return transferEncodingChunked; }
};