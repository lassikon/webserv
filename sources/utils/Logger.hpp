#pragma once

#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <Colors.hpp>

#define LOGFILE "webserv.log"

class Logger {
private:
  enum class logLevel { Trace, Debug, Info, Warn, Error, Critical };
  enum class logOutput { ConsoleOnly, FileOnly, Both };

  logLevel currentLevel;
  logOutput currentOutput;
  std::ofstream logFile;

  std::string levelToString(logLevel Level) const;
  std::string logTimeStamp(void) const;

  static Logger &newLogInstance(void) {
    static Logger logger;
    return logger;
  }

public:
  Logger(void);
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;
  ~Logger(void);

  void setLogLevel(logLevel newLevel);
  void setLogOutput(logOutput newOutput);
  void createLogFile(void);
  void closeLogFile(void);

private:
  template <typename... Args> void log(logLevel logLvl, Args... args) {
    if (logLvl < currentLevel)
      return;
    std::ostringstream logEntry;
    logEntry << "[" << levelToString(logLvl) << "]"
             << "[" << logTimeStamp() << "] ";
    (logEntry << ... << args);
    if (currentOutput != logOutput::FileOnly)
      std::cerr << logEntry.str() << std::endl;
    if (currentOutput != logOutput::ConsoleOnly && logFile.is_open())
      logFile << logEntry.str() << std::endl;
  }

public:
  template <typename... Args> static void Debug(Args... args) {
    newLogInstance().log(logLevel::Debug, args...);
  }
};

#define LOG_DEBUG(...) (Logger::Debug(__VA_ARGS__))
