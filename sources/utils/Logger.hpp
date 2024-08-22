#pragma once

#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <Colors.hpp>

enum class logLevel { Trace, Debug, Info, Warn, Error, Critical };
enum class logDetail { TimeStamp, SourceFile, LineNumber };
enum class logOutput { ConsoleOnly, FileOnly, Both };

class Logger {
private:
  const char *logFileName = "webserv.log";
  std::ofstream logFile;
  bool enabledDetail[3];

  logLevel currentLevel;
  logOutput currentOutput;

  void createLogFile(void);
  void closeLogFile(void);

  std::vector<std::string> getLogInfo(logLevel Level);
  std::string getTimeStamp(void) const;
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
  void setLogDetails(bool time, bool file, bool line);

private:
  template <typename... Args>
  void create(logLevel logLvl, int lineNbr, const char *srcFile, Args... args) {
    if (logLvl < currentLevel)
      return;
    std::ostringstream logEntry;
    std::vector<std::string> logInfo = getLogInfo(logLvl);
    logEntry << logInfo[1] << "[" << logInfo[0] << "]";
    if (enabledDetail[(int)logDetail::TimeStamp])
      logEntry << "[" << getTimeStamp() << "]";
    if (enabledDetail[(int)logDetail::SourceFile])
      logEntry << "[" << srcFile << "]";
    if (enabledDetail[(int)logDetail::LineNumber])
      logEntry << "[line:" << lineNbr << "]";
    logEntry << " ";
    (logEntry << ... << args);
    if (currentOutput != logOutput::FileOnly)
      std::cerr << logEntry.str() << std::endl;
    if (currentOutput != logOutput::ConsoleOnly && logFile.is_open())
      logFile << logEntry.str() << std::endl;
  }

public:
  template <typename... Args>
  static void Log(logLevel logLvl, int lineNbr, const char *srcFile, Args... args) {
    newLogInstance().create(logLvl, lineNbr, srcFile, args...);
  }
};

#define LOG_TRACE(...) (Logger::Log(logLevel::Trace, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_DEBUG(...) (Logger::Log(logLevel::Debug, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_INFO(...) (Logger::Log(logLevel::Info, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_WARNING(...) (Logger::Log(logLevel::Warn, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_ERROR(...) (Logger::Log(logLevel::Error, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_CRITICAL(...) (Logger::Log(logLevel::Critical, __LINE__, __FILE__, __VA_ARGS__))
