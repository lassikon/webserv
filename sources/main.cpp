#include <Config.hpp>
#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <Server.hpp>

int g_SignalReceived;
int g_ExitStatus;

int main(int argc, char **argv) {
  if (argc > 2) {
    LOG_INFO(ERR_MSG_USAGE);
    return (int)Error::Args;
  } else if (argc == 2) {
    Config config(argv[1]);
  } else {
    Config config(void);
  }
  return g_ExitStatus;
}

/* class Test { */
/* public: */
/*   void checkArgCount(int argc) { */
/*     (void)argc; */
/*     std::string a = std::string("abc").substr(10); */
/*     std::string fileName = "does_not_exist.txt"; */
/*     std::ifstream f(fileName); */
/*     if (!f) */
/*       THROW(Error::Config, ERR_MSG_NOFILE, fileName, STRERROR); */
/*     LOG_INFO("This line would execute without throw!"); */
/*   } */
/* }; */

/* int main(int argc, char **argv) { */
/*   Exception::tryCatch(&Config::Parse, &config); */
/*   if (!config.getAvailableServers) { */
/*     LOG_WARN(ERR_MSG_NOSERVER, config.getFileName); */
/*     return (int)Error::Config; */
/*   } */
/*   Server server(config.getServerConfigs); */
/*   Exception::tryCatch(&Server::Run, &server); */
/*   return g_ExitStatus: */
/* } */
