#include <CgiHandler.hpp>
#include <Config.hpp>
#include <ConfigInitializer.hpp>
#include <Exception.hpp>
#include <Global.hpp>
#include <Logger.hpp>
#include <Server.hpp>
#include <ServerManager.hpp>
#include <Signal.hpp>
#include <UrlEncoder.hpp>

sig_atomic_t g_ExitStatus;
std::vector<struct CgiParams> g_CgiParams;

void encoderTesting() {
  std::string str = "!Hello, World!";
  std::string encoded = UrlEncoder::encode(str);
  std::string decoded = UrlEncoder::decode(encoded);
  LOG_INFO("Original string:", str);
  LOG_INFO("Encoded string:", encoded);
  LOG_INFO("Decoded string:", decoded);

  LOG_INFO("========================================");

  str = "Jarno is _the_ ~man~!";
  encoded = UrlEncoder::encode(str);
  decoded = UrlEncoder::decode(encoded);
  LOG_INFO("Original string:", str);
  LOG_INFO("Encoded string:", encoded);
  LOG_INFO("Decoded string:", decoded);
}

int main(int argc, char** argv) {
  if (argc > 2) {
    LOG_INFO(ERR_MSG_USAGE);
    return (int)Error::Args;
  }
  Signal::trackSignals();
  encoderTesting();
  (void)argv;
  /* ======================================================================= */
  /* Config config; */
  /* if (argc == 2) { */
  /*   config.setFilePath(argv[1]); */
  /* } */
  /* ======================================================================= */

  //MY TESTING
  // CgiHandler cgi;
  // cgi.runScript();

  /* ======================================================================= */

  // Config config = ConfigInitializer::initializeConfig(argc, argv);
  // Exception::tryCatch(&Config::parseConfigFile, &config);
  // if (config.getServers().empty()) {
  //   LOG_FATAL(ERR_MSG_NOSERVER, config.getFilePath());
  //   return (int)Error::Config;
  // }

  // config.printServerConfig();
  // ServerManager serverManager;
  // serverManager.configServers(config);
  // serverManager.runServers();

  /* ======================================================================= */
  /* ServerManager server; */
  /* Exception::tryCatch(&ServerManager::configServers, &server, config); */
  /* if (config.getServers().empty()) { */
  /*   LOG_FATAL(ERR_MSG_SERVER, config.getFileName); */
  /*   return (int)Error::Server; */
  /* } */
  /* Exception::tryCatch(&ServerManager::runServers, &server); */
  /* ======================================================================= */

  return g_ExitStatus;
}
