#pragma once

#include <Logger.hpp>
#include <Response.hpp>
#include <Typedef.hpp>
#include <Utility.hpp>

#include <chrono>
#include <cstdlib>
#include <string>
#include <unordered_map>

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
  // const std::chrono::system_clock::time_point lifetime = std::chrono::seconds(3600);
  // std::chrono::time_point<typename Clock> lifetime = 3600;  // in seconds
  // the key should be the token and value would be like an object or struct
  // this struct would have data which is client info or preferences like cookie details us just name and cookie for post request
  // for simple get request the value would be some default
  // but in a real thing it would be a database with user preferences

  const std::chrono::seconds lifetime;
  std::unordered_map<std::string, std::chrono::system_clock::time_point> sessionIds{};

 private:
  std::string setExpireTime(void);
  void debugPrintSessionsMap(void);

 public:
  SessionManager(void);
  ~SessionManager(void);

 public:  // setters
  std::string setSessionCookie(void);

 public:  // getters
  bool isSessionCookie(std::string sessionToken);
};
