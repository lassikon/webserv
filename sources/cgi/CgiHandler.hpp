#pragma once

#include <Exception.hpp>
#include <Logger.hpp>
#include <Utility.hpp>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

class CgiHandler {
 private:
  enum Fd { Read, Write };

  int pipefd[2];
  int wstat;
  pid_t pid;

  int tempfd;  // this should be replaced

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
  bool validCgiFile(void);
};
