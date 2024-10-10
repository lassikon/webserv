#pragma once

#include <Logger.hpp>
#include <Response.hpp>
#include <Utility.hpp>

#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

class Server;

class SessionManager {
 private:
  const int tokenLength = 20;
  const std::string charSet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789";

  // struct Cookie {
  //   std::string token;
  //   std::string body;
  // };
  // std::vector<std::shared_ptr<Cookie>> cookies;

  // the key should be the token and value would be like an object or struct
  // this struct would have data which is client info or preferences like cookie details us just name and cookie for post request
  // for simple get request the value would be some default
  // but in a real thing it would be a database with user preferences

  std::unordered_map<std::string, std::string> sessionIds{};

  Server& server;
  std::vector<std::shared_ptr<Client>> clients;

 private:
  std::string setExpireTime(void);

 public:
  SessionManager(void) = delete;
  SessionManager(Server& server);
  ~SessionManager(void);

 public:  // setters
  std::string setSessionCookie(void);

 public:  // getters
  std::string getSessionCookie(std::string sessionToken);
  bool isSessionCookie(std::string sessionToken);
};
