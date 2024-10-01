#pragma once

#include <Logger.hpp>
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

  std::fstream sessionsFile;
  std::unordered_map<std::string, std::string> sessionIds{};

 public:
  SessionManager(void);
  ~SessionManager(void);

 public:
  void generateOutfile(std::fstream& fs, const char* file);
  void readSessionsFromFile(void);
  std::string randomizeSessionToken(void);

 public:
  void debugFillSessionsFile();
  void debugPrintSessionsMap();

 public:  // setters
  std::string setSessionCookie(void);

 public:  // getters
  SessionManager& getSessionManager(void) { return *this; }
  std::string getSessionCookie(std::string);
  std::string getSessionQuery(std::string);
};
