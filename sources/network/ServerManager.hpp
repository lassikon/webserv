#pragma once

#include <CgiHandler.hpp>
#include <Config.hpp>
#include <Global.hpp>
#include <IException.hpp>
#include <Logger.hpp>
#include <PollManager.hpp>
#include <RuntimeException.hpp>
#include <Server.hpp>
#include <Typedef.hpp>

#include <sys/epoll.h>
#include <sys/wait.h>

#include <chrono>
#include <memory>
#include <vector>

class ServerManager {

#define CHILD_TIMEOUT 10

 private:
  std::vector<std::shared_ptr<Server>> servers;
  
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
  void handlePollErrors(PollManager& pollManager, struct epoll_event& event);
  void handlePollInEvent(PollManager& pollManager, struct epoll_event& event);
  void handlePollOutEvent(PollManager& pollManager, struct epoll_event& event);
  void handleNoEvents(PollManager& pollManager);

};
