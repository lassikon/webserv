#pragma once

#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <Colors.hpp>

#define LOGFILE "webserv.log"

enum class logLevel { Trace, Debug, Info, Warn, Error, Critical };
enum class logDetail { None, TimeStamp, SourceFile, LineNumber, All };
enum class logOutput { ConsoleOnly, FileOnly, Both };

class Logger {
private:
  logLevel currentLevel;
  logOutput currentOutput;
  logDetail currentDetail;
  std::ofstream logFile;

  std::string levelToString(logLevel Level) const;
  std::string logTimeStamp(void) const;
  void createLogFile(void);
  void closeLogFile(void);

  static Logger &getLogInstance(void) {
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
  void setLogDetail(logDetail newDetail);

private:
  template <typename... Args>
  void output(logLevel logLvl, int lineNbr, const char *srcFile, Args... args) {
    if (logLvl < currentLevel)
      return;
    std::ostringstream logEntry;
    logEntry << "[" << logTimeStamp() << "]"
             << "[" << srcFile << "]"
             << "[LINE: " << lineNbr << "]"
             << "[" << levelToString(logLvl) << "] ";
    (logEntry << ... << args);
    if (currentOutput != logOutput::FileOnly)
      std::cerr << logEntry.str() << std::endl;
    if (currentOutput != logOutput::ConsoleOnly && logFile.is_open())
      logFile << logEntry.str() << std::endl;
  }

public:
  template <typename... Args>
  static void Log(logLevel logLvl, int lineNbr, const char *srcFile,
                  Args... args) {
    getLogInstance().output(logLvl, lineNbr, srcFile, args...);
  }
};

#define LOG_TRACE(...)                                                         \
  (Logger::Log(logLevel::Trace, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_DEBUG(...)                                                         \
  (Logger::Log(logLevel::Debug, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_INFO(...)                                                          \
  (Logger::Log(logLevel::Info, __LINE__, __FILE__, __VA_ARGS__))
