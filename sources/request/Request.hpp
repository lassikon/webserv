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
  std::string reqQuery;
  std::string reqCookie; // is this needed?
  std::map<std::string, std::string> reqHeaders;
  std::vector<char> reqBody;
  size_t reqBodySize = 0;

 public:
  Request();
  ~Request();

 public:  // getters
  std::map<std::string, std::string>& getHeaders(void) { return reqHeaders; }
  std::string getMethod(void) const { return reqMethod; }
  std::string getReqURI(void) const { return reqURI; }
  std::string getVersion(void) const { return reqVersion; }
  std::string getQuery(void) const { return reqQuery; }
  std::string getCookie(void) const { return reqCookie; } // is this needed?
  size_t getBodySize(void) const { return reqBodySize; }
  std::map<std::string, std::string> getHeaders(void) const { return reqHeaders; }
  std::vector<char> getBody(void) const { return reqBody; }

 public:  // setters
  void setMethod(std::string method) { reqMethod = method; }
  void setReqURI(std::string uri) { reqURI = uri; }
  void setVersion(std::string version) { reqVersion = version; }
  void setBody(std::vector<char> body) { reqBody = std::move(body); }
  void setBodySize(size_t size) { reqBodySize = size; }
  void setQuery(std::string query) { reqQuery = query; }
  void setCookie(std::string cookie) { reqCookie = cookie; } // is this needed?
};
