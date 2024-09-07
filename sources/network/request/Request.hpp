#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <Logger.hpp>
#include <Utility.hpp>

class Client;

class Request {
 private:
  std::string reqMethod;
  std::string reqURI;
  std::string reqVersion;
  std::map<std::string, std::string> reqHeaders;
  std::vector<char> reqBody;
  size_t reqBodySize = 0;
  bool transferEncodingChunked;

 public:
  Request();
  ~Request();

  std::map<std::string, std::string>& getHeaders(void) { return reqHeaders; }

  void parseRequestLine(Client* client, std::string& requestLine);
  void parseHeaders(Client* client, std::istringstream& iBuf);
  void parseBody(Client* client, std::istringstream&, int nbytes);
  void parseChunkedBody(Client* client, std::istringstream& iBuf);

  std::string getMethod(void) const;
  std::string getReqURI(void) const;
  std::string getVersion(void) const;
  size_t getBodySize(void) const;
  std::map<std::string, std::string> getHeaders(void) const;
  std::vector<char> getBody(void) const;
  bool isTransferEncodingChunked(void) const;
};