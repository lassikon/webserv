#pragma once

#include <Exception.hpp>
#include <Logger.hpp>
#include <Request.hpp>
#include <Typedef.hpp>
#include <Utility.hpp>

#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <string>
#include <vector>

class CgiHandler {
 private:
  static std::vector<pid_t> pids;

 private:
  enum Fd { Read, Write };

  std::string cgi;
  ENVMAP envp;
  pid_t pid;

  int pipefd[2];
  int sockfd;
  int wstat;

 public:
  CgiHandler(const Request& request);
  ~CgiHandler(void);

 public:
  void runScript(void);

 private:
  void scriptLoader(void);
  void forkChildProcess(void);
  void waitChildProcess(void);
  void executeCgiScript(void);
  void setEnvParams(void);
  void closePipeFds(void);

 private:
  bool isAccessable(void) const;
  bool isParentProcess(void) const;
  bool isChildProcess(void) const;

 private:
  static inline const pid_t& addNewProcessId(void);

 public:
  static void killAllChildPids(void);

 private:
  template <typename... Args> void cgiError(Args&&... args) {
    THROW(Error::Cgi, std::forward<Args>(args)...);
  }
};
