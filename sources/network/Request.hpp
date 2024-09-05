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
  std::string method;
  std::string reqURI;
  std::string version;
  std::map<std::string, std::string> headers;
  std::string body;
  size_t bodySize = 0;
  bool transferEncodingChunked;

 public:
  Request(void) {transferEncodingChunked = false;}
  ~Request(void) {}

  std::map<std::string, std::string>& getHeaders(void) { return headers; }

  void parseRequestLine(Client* client, std::string& requestLine);
  void parseHeaders(Client* client, std::istringstream& iBuf);
  void parseBody(Client* client, std::istringstream& , int nbytes);
  void parseChunkedBody(Client* client, std::istringstream& iBuf);

  std::string getMethod(void) const { return method; }
  std::string getReqURI(void) const { return reqURI; }
  std::string getVersion(void) const { return version; }
  size_t getBodySize(void) const { return bodySize; }
  std::map<std::string, std::string> getHeaders(void) const { return headers; }
  std::string getBody(void) const { return body; }
  bool isTransferEncodingChunked(void) const { return transferEncodingChunked; }
};