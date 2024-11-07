#pragma once

#include <Logger.hpp>
#include <Response.hpp>
#include <Typedef.hpp>
#include <Utility.hpp>

#include <chrono>
#include <cstdlib>
#include <random>
#include <string>
#include <unordered_map>

#define COOKIE_TIMEOUT 3600  // in seconds

  // in real case, there key should be a token and value like an object or struct
  // this struct would have data which is client info or user preferences
  // backend would be a database with user information

class SessionManager {
 private:
  cookiemap_t sessionIds{};
  const std::chrono::seconds lifetime;
  const size_t tokenLength = 20;
  const std::string charSet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789";

 public:
  SessionManager(void);
  ~SessionManager(void);

 public:
  std::string setSessionCookie(void);
  void checkExpiredCookies(void);
  bool isSessionCookie(std::string sessionToken);

 private:
  std::string setExpireTime(void);
  void debugPrintTokens(void);
};
