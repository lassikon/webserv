#pragma once

#include <Client.hpp>
#include <Exception.hpp>
#include <Logger.hpp>
#include <Typedef.hpp>
#include <Utility.hpp>

#include <sys/stat.h>
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

  std::vector<std::string> envps{};
  std::vector<std::string> args{};

  int pipefd[2];
  int wstat;
  pid_t pid;

  ENVPMAP envpmap;

  // probably delete these?
  std::string cgi;
  int sockfd;

 public:
  CgiHandler(void) = delete;
  CgiHandler(const Client& client);
  ~CgiHandler(void);

 public:
  void runScript(void);

 private:
  void scriptLoader(void);
  void generateEnvpMap(void);
  void forkChildProcess(void);
  void waitChildProcess(void);
  void executeCgiScript(void);
  void closePipeFds(void);

 private:
  std::vector<char*> createArgvArray(void);
  std::vector<char*> createEnvpArray(void);
  void convertEnvpMapToVector(void);

 private:
  bool isValidScript(void) const;
  bool isParentProcess(void) const;
  bool isChildProcess(void) const;

 private:
  static inline const pid_t& addNewProcessId(void) noexcept;

 public:
  static void killAllChildPids(void);

 private:
  template <typename... Args> void cgiError(Args&&... args) {
    THROW(Error::Cgi, std::forward<Args>(args)...);
  }
};
