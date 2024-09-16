#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <Logger.hpp>
#include <Utility.hpp>

class Request {
 private:
  std::string reqMethod;
  std::string reqURI;
  std::string reqVersion;
  std::map<std::string, std::string> reqHeaders;
  std::vector<char> reqBody;
  size_t reqBodySize = 0;

 public:
  Request();
  ~Request();

  std::map<std::string, std::string>& getHeaders(void) { return reqHeaders; }
  std::string getMethod(void) const { return reqMethod; }
  std::string getReqURI(void) const { return reqURI; }
  std::string getVersion(void) const { return reqVersion; }
  size_t getBodySize(void) const { return reqBodySize; }
  std::map<std::string, std::string> getHeaders(void) const { return reqHeaders; }
  std::vector<char> getBody(void) const { return reqBody; }
  void setMethod(std::string method) { reqMethod = method; }
  void setReqURI(std::string uri) { reqURI = uri; }
  void setVersion(std::string version) { reqVersion = version; }
  void setBody(std::vector<char>& body) { reqBody = body; }
  void setBodySize(size_t size) { reqBodySize = size; }
};