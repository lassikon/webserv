#pragma once

#include <Exception.hpp>
#include <Logger.hpp>
#include <Utility.hpp>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <vector>

class CgiHandler {
 private:
  enum Fd { Read, Write };

  static std::vector<pid_t> pids;
  pid_t pid;

  int pipefd[2];
  int sockfd;
  int wstat;

 public:
  CgiHandler(void);
  ~CgiHandler(void);

 public:
  void cgiLoader(void);

 private:
  void createChildProcess(void);
  void waitChildProcess(void);
  void executeCgiScript(void);
  void setEnvParams(void);
  void closePipeFds(void);

 private:
  bool validCgiAccess(void) const;
  bool validCgiScript(void) const;
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
