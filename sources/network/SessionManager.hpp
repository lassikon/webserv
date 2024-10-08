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
  const std::string charSet =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789";

  const char* fileName = "sessions";
  const int tokenLength = 20;
  bool errorLogged = false;

  std::fstream sessionsFile;
  std::unordered_map<std::string, std::string> sessionIds{};
  
  Server &server;
  std::vector<std::shared_ptr<Client>> clients;

 public:
  SessionManager(void) = delete;
  SessionManager(Server &server);
  ~SessionManager(void);

 public:
  void generateOutfile(std::fstream& fs, const char* file);
  void readSessionsFromFile(void);

 public:
  void debugFillSessionsFile();
  void debugPrintSessionsMap();

 public:  // setters
  std::string setSessionCookie(Response& response);

 public:  // getters
  SessionManager& getSessionManager(void) { return *this; }
  std::string getSessionCookie(std::string);
  std::string getSessionQuery(std::string);
};
