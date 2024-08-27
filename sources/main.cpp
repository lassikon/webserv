#include "Logger.hpp"
#include <Config.hpp>
#include <Exception.hpp>
#include <Global.hpp>
#include <Server.hpp>

int g_SignalReceived;
int g_ExitStatus;

class Test {
public:
  void callChecker(int argc) { privateFunc(argc); };
  void checkArgCount(int argc) {
    if (argc)
      THROW_WARN(ERR_MSG_USAGE);
    LOG_INFO("This line would execute without throw!");
  }

private:
  void privateFunc(int argc) {
    Exception::tryCatch(&Test::checkArgCount, this, argc);
  }
};


int main(int argc, char **argv) {
  if (argc > 2) {
    LOG_INFO(ERR_MSG_USAGE);
    return (int)ErrorCode::ArgCount;
  } else if (argc == 2) {
    Config config(argv[1]);
  } else {
    Config config(void);
  }

  // TESTING
  Test t;
  t.callChecker(argc);
  Exception::tryCatch(&Test::checkArgCount, &t, argc);
  LOG_INFO("This line will get executed, once stack is unwinded!");
}

// rest of main
/* int main(int argc, char **argv) {
  Exception::tryCatch(&Config::Parse, &config);
  if (!config.getAvailableServers) {
    LOG_WARN(ERR_MSG_NOSERVER, config.getFileName);
    return (int)ErrorCode::ConfigFile;
  }
  Server server(config.getServerConfigs);
  Exception::tryCatch(&Server::Run, &server);
  return g_ExitStatus:
}*/
