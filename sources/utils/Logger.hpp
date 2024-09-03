#pragma once

#include <Colors.hpp>

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

enum class logLevel { Trace, Debug, Info, Warn, Error, Fatal };

class Logger {
 private:
  enum class logOutput { ConsoleOnly, FileOnly, Both };
  enum class logDetail { Time, File, Func, Line };
  const char* fileName = "webserv.log";
  std::ofstream logFile;
  logOutput currentOutput;
  logLevel currentLevel;
  bool enabledDetail[4];

 public:
  Logger(void);
  ~Logger(void);

 private:
  void loadDefaults(void);
  void createLogFile(void);
  void closeLogFile(void);
  std::string getDateTimeStamp(void);
  void setLogDetails(bool time, bool file, bool func, bool line);
  void setLogDetail(int index, bool value);

 private:
  static inline Logger& newLogInstance(void) noexcept {
    static Logger logger;
    return logger;
  }

 private:
  template <typename... Args>
  void create(logLevel levelLog, const char* levelTitle, const char* levelColor,
              std::ostream& consoleStream, const char* sourceFile, const char* functionName,
              int lineNumber, Args&&... args) {
    if (levelLog < currentLevel) {
      return;
    }
    std::ostringstream logEntry;
    logEntry << levelColor << "[" << levelTitle << "]";
    if (enabledDetail[(int)logDetail::Time]) {
      logEntry << "[" << getDateTimeStamp() << "]";
    }
    if (enabledDetail[(int)logDetail::File]) {
      logEntry << "[" << sourceFile;
    }
    if (enabledDetail[(int)logDetail::Func]) {
      logEntry << "/" << functionName;
    }
    if (enabledDetail[(int)logDetail::Line]) {
      logEntry << ":" << lineNumber << "]";
    }
    ([&] { logEntry << " " << args; }(), ...);
    if (currentOutput != logOutput::FileOnly) {
      consoleStream << logEntry.str() << RESET << std::endl;
    }
    if (currentOutput != logOutput::ConsoleOnly && logFile.is_open()) {
      logFile << logEntry.str() << RESET << std::endl;
    }
  }

 public:
  template <typename... Args>
  static void Log(logLevel levelLog, const char* levelTitle, const char* levelColor,
                  std::ostream& consoleStream, const char* sourceFile, const char* functionName,
                  int lineNumber, Args&&... args) {
    newLogInstance().create(levelLog, levelTitle, levelColor, consoleStream, sourceFile,
                            functionName, lineNumber, args...);
  }
};

#define LOG_TRACE(...)                                                                  \
  (Logger::Log(logLevel::Trace, "TRACE", CYAN, std::cout, __FILE__, __func__, __LINE__, \
               __VA_ARGS__))
#define LOG_DEBUG(...)                                                                   \
  (Logger::Log(logLevel::Debug, "DEBUG", GREEN, std::cout, __FILE__, __func__, __LINE__, \
               __VA_ARGS__))
#define LOG_INFO(...) \
  (Logger::Log(logLevel::Info, "INFO", BLUE, std::cout, __FILE__, __func__, __LINE__, __VA_ARGS__))
#define LOG_WARN(...)                                                                      \
  (Logger::Log(logLevel::Warn, "WARNING", YELLOW, std::cout, __FILE__, __func__, __LINE__, \
               __VA_ARGS__))
#define LOG_ERROR(...) \
  (Logger::Log(logLevel::Error, "ERROR", RED, std::cerr, __FILE__, __func__, __LINE__, __VA_ARGS__))
#define LOG_FATAL(...) \
  (Logger::Log(logLevel::Fatal, "FATAL", RED, std::cerr, __FILE__, __func__, __LINE__, __VA_ARGS__))
