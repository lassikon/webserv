#pragma once

#include <Utility.hpp>

#include <cstdlib>
#include <fstream>
#include <string>
#include <unordered_map>

class SessionManager {
 private:
  const std::string charSet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789";

  const char* fileName = "sessions";
  const int tokenLength = 20;

  std::fstream sessions;
  std::unordered_map<std::string, std::string> sessionIds{};

 public:
  SessionManager(void);
  ~SessionManager(void);

 private:
  void generateOutfile(std::fstream& fs, const char* file);
  std::string randomizeSessionToken(void);

 public:
  std::string generateSessionId(void);
  bool sessionIdExists(std::string);

 public:  // getters
  SessionManager& getSessionManager(void) { return *this; }
  std::string getSessionId(std::string);
};
