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
  std::string path;
  std::string version;
  std::map<std::string, std::string> headers;
  std::string body;
  bool transferEncodingChunked;
  std::shared_ptr<Config> config;

 public:
  Request(void) {transferEncodingChunked = false;}
  ~Request(void) {}

  void parseRequestLine(Client* client, std::string& requestLine);
  void parseHeaders(Client* client, std::istringstream& iBuf);
  void parseBody(Client* client, std::istringstream& iBuf);

  std::string getMethod(void) const { return method; }
  std::string getPath(void) const { return path; }
  std::string getVersion(void) const { return version; }
  std::map<std::string, std::string> getHeaders(void) const { return headers; }
  std::string getBody(void) const { return body; }
  bool isTransferEncodingChunked(void) const { return transferEncodingChunked; }

  void setConfig(std::shared_ptr<Config> config) { this->config = config; }
};