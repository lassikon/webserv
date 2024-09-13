#pragma once

#include <poll.h>
#include <sys/wait.h>

#include <CgiHandler.hpp>
#include <Config.hpp>
#include <Global.hpp>
#include <PollManager.hpp>
#include <Server.hpp>
#include <string>
#include <vector>

class ServerManager {

#define CHILD_TIMEOUT 10

 private:
  std::vector<std::shared_ptr<Server>> servers;
  std::vector<struct pollfd> pollFds;
  // std::size_t pidsMapSize;

 public:
  ServerManager(void);
  ~ServerManager(void);

  void runServers(void);
  void configServers(Config& config);

 private:
  void serverLoop(PollManager& pollManager);
  bool checkServerExists(ServerConfig& serverConfig);
  void checkChildProcesses(PollManager& pollManager);
  void checkForNewChildProcesses(PollManager& pollManager);
  bool childTimeout(std::chrono::time_point<std::chrono::steady_clock>& start);
  void initializePollManager(PollManager& pollManager);
  void handleNoEvents(PollManager& pollManager);
  bool handlePollErrors(PollManager& pollManager, struct pollfd& pollFd);
  void handlePollInEvent(PollManager& pollManager, struct pollfd& pollFd);
  void handlePollOutEvent(PollManager& pollManager, struct pollfd& pollFd);

  bool isCgiFd(int fd) const;
  int getClientFdFromCgiParams(int fd) const;

  template <typename... Args> void serverError(Args&&... args) {
    THROW(Error::Server, std::forward<Args>(args)..., STRERROR);
  }
};
