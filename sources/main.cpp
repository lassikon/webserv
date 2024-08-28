#include <Config.hpp>
#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <Server.hpp>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>

int g_SignalReceived;
int g_ExitStatus;

class Test {
public:
  void checkArgCount(int argc) {
    (void)argc;

    // normal errno is not catched by c++ exception
    double not_valid = std::log(-1.0);
    LOG_INFO(not_valid, errno);

    // testing infile
    std::string fileName = "does_not_exist.txt";
    std::ifstream f(fileName);
    if (!f)
      THROW(Error::Config, fileName, strerror(errno));

    // c++ exceptions
    std::string a = std::string("abc").substr(10);

    LOG_INFO("This line would execute without throw!");
  }
};

int main(int argc, char **argv) {
  if (argc > 2) {
    LOG_INFO(ERR_MSG_USAGE);
    return (int)Error::Args;
  } else if (argc == 2) {
    Config config(argv[1]);
  } else {
    Config config(void);
  }
  Test t;
  Exception::tryCatch(&Test::checkArgCount, &t, argc);
  LOG_INFO("This line will get executed, once stack is unwinded in main!");
  return g_ExitStatus;
}

// rest of main
/* int main(int argc, char **argv) {
  Exception::tryCatch(&Config::Parse, &config);
  if (!config.getAvailableServers) {
    LOG_WARN(ERR_MSG_NOSERVER, config.getFileName);
    return (int)Error::Config;
  }
  Server server(config.getServerConfigs);
  Exception::tryCatch(&Server::Run, &server);
  return g_ExitStatus:
}*/
