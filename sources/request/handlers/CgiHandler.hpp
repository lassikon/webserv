#pragma once

#include <Client.hpp>
#include <Config.hpp>
#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
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

struct CgiParams {
  pid_t pid;
  int fd;
  std::chrono::time_point<std::chrono::steady_clock> start;
};

class CgiHandler {
 private:
  // static std::vector<pid_t> pids;
  // static std::map<pid_t, int> pids;

 private:
  enum Fd { Read, Write };

  std::vector<std::string> envps{};
  std::vector<std::string> args{};
  std::string cgi;  // delete this?

  int pipefd[2];
  int cgiFd;
  int wstat;
  pid_t pid;

 public:
  /* CgiHandler(void) = delete; */
  CgiHandler();  // delete this
  CgiHandler(const Client& client);
  ~CgiHandler(void);

 public:
  void runScript(void);

 private:
  void scriptLoader(void);
  void generateEnvpVector(void);
  void forkChildProcess(void);
  void waitChildProcess(void);
  void executeCgiScript(void);
  void closePipeFds(void);
  void debugPrintCgiFd(void);

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
