#pragma once

#include <IRequestHandler.hpp>
#include <Logger.hpp>
#include <RuntimeException.hpp>

#include <Typedef.hpp>
#include <Utility.hpp>

#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

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
  bool isExited = false;
  steady_time_point_t start;
};

class CgiHandler : public IRequestHandler {
 private:
  // static std::vector<pid_t> pids;
  // static std::map<pid_t, int> pids;

 private:
  enum Fd { Read, Write };

  std::vector<std::string> envps{};
  std::vector<std::string> args{};
  std::string cgi;  // delete this?

  int outPipeFd[2];
  int inPipeFd[2];
  int clientFd;
  int wstat;
  pid_t pid;

 public:
  /* CgiHandler(void) = delete; */
  CgiHandler();  // delete this
  CgiHandler(const Client& client);
  virtual ~CgiHandler(void);

 public:
  void executeRequest(Client& client) override;
  void runScript(void);

 private:
  void scriptLoader(void);
  void setGlobal(void); 
  void generateEnvpVector(Client& client);
  void forkChildProcess(void);
  void executeCgiScript(void);
  void closePipeFds(void);

 private:
  std::vector<char*> convertStringToChar(std::vector<std::string>& vec);

 private:
  bool isValidScript(void) const;
  bool isParentProcess(void) const;
  bool isChildProcess(void) const;

 private:
  static inline const pid_t& addNewProcessId(void) noexcept;

 public:
  static void killAllChildPids(void);
};
