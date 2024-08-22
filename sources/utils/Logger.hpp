#pragma once

#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <Colors.hpp>

#define LOGFILE "webserv.log"

enum class logLevel { Trace, Debug, Info, Warn, Error, Critical };
enum class logDetail { None, Time, Time_File, Time_File_Line, All };
enum class logOutput { ConsoleOnly, FileOnly, Both };

class Logger {
private:
  logLevel currentLevel;
  logOutput currentOutput;
  logDetail currentDetail;
  std::ofstream logFile;

  void createLogFile(void);
  void closeLogFile(void);

  std::string levelToString(logLevel Level) const;
  std::string logTimeStamp(void) const;
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
  void create(logLevel logLvl, int lineNbr, const char *srcFile, Args... args) {
    if (logLvl < currentLevel)
      return;
    std::string color = GREEN;
    std::ostringstream logEntry;
    logEntry << color << "[" << levelToString(logLvl) << "]";
    logEntry << "[" << logTimeStamp() << "]"
             << "[" << srcFile << "]"
             << "[line:" << lineNbr << "]";
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
    getLogInstance().create(logLvl, lineNbr, srcFile, args...);
  }
};

#define LOG_TRACE(...)                                                         \
  (Logger::Log(logLevel::Trace, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_DEBUG(...)                                                         \
  (Logger::Log(logLevel::Debug, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_INFO(...)                                                          \
  (Logger::Log(logLevel::Info, __LINE__, __FILE__, __VA_ARGS__))
