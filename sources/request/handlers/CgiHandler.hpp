#pragma once

#include <IRequestHandler.hpp>
#include <Logger.hpp>
#include <RuntimeException.hpp>

#include <Typedef.hpp>
#include <Utility.hpp>

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <filesystem>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <string>
#include <vector>

class Client;

struct CgiParams {
  pid_t pid = -1;
  int outReadFd = -1;
  int outWriteFd = -1;
  int inReadFd = -1;
  int inWriteFd = -1;
  int clientFd = -1;
  int childExitStatus = -1;
  bool isExited = false;
  bool isTimeout = false;
  bool isFailed = false;
  steady_time_point_t start;
};

class CgiHandler : public IRequestHandler {
 private:
  enum Fd { Read, Write };

  std::vector<std::string> envps{};
  std::vector<std::string> args{};

  std::vector<char*> convertStringToChar(std::vector<std::string>& vec);

  int outPipeFd[2] = {-1, -1};
  int inPipeFd[2] = {-1, -1};
  int clientFd;

  std::string cgi;
  bool isBin = false;
  pid_t pid;

 public:
  CgiHandler();
  virtual ~CgiHandler(void);

 public:
  void executeRequest(Client& client) override;
  void closePipeFds(void);

 private:
  void scriptLoader(Client& client);
  void setGlobal(void);
  void generateEnvpVector(Client& client);
  void forkChildProcess(Client& client);
  void executeCgiScript(Client& client);
  void exitError(int status, const std::string& message);

 private:
  bool isValidFile(void) const;
  bool isValidPerm(void) const;
  bool isParentProcess(void) const;
  bool isChildProcess(void) const;

 public:
  static void killAllChildPids(void);
};
