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

  // in real case, the key should be a token and value like an object or struct
  // this struct would have data which is client info or preferences
  // backend would be a database with user information

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
  void checkExpiredCookies(void);
};
