#pragma once

#include <CgiHandler.hpp>
#include <Config.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <PollManager.hpp>
#include <Server.hpp>
#include <Typedef.hpp>

#include <poll.h>
#include <sys/wait.h>

#include <chrono>
#include <memory>
#include <vector>

class ServerManager {

#define CHILD_TIMEOUT 10

 private:
  std::vector<std::shared_ptr<Server>> servers;
  std::vector<struct pollfd> pollFds;

 public:
  ServerManager(void);
  ~ServerManager(void);

  void runServers(void);
  void configServers(Config& config);

 private:
  void serverLoop(PollManager& pollManager);
  bool checkServerExists(ServerConfig& serverConfig);
  void initializePollManager(PollManager& pollManager);

 private:  
  void checkChildProcesses(PollManager& pollManager);
  void checkForNewChildProcesses(PollManager& pollManager);
  bool childTimeout(steady_time_point_t& start);

 private:
  bool handlePollErrors(PollManager& pollManager, struct pollfd& pollFd);
  void handlePollInEvent(PollManager& pollManager, struct pollfd& pollFd);
  void handlePollOutEvent(PollManager& pollManager, struct pollfd& pollFd);
  void handleNoEvents(PollManager& pollManager);

 private:
  bool isCgiFd(int fd) const;
  int getClientFdFromCgiParams(int fd) const;
  int getCgiFdFromClientFd(int fd) const;
};
