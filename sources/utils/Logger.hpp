#pragma once

#include <cerrno>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <Colors.hpp>

enum class logLevel { Trace, Debug, Info, Warn, Error, Fatal };

class Logger {
private:
  const char *fileName = "webserv.log";
  std::ofstream logFile;

  enum class logDetail { TimeStamp, SourceFile, LineNumber };
  enum class logOutput { ConsoleOnly, FileOnly, Both };

  logLevel currentLevel;
  logOutput currentOutput;
  bool enabledDetail[3];

public:
  Logger(void);
  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;
  ~Logger(void);

private:
  void createLogFile(void);
  void closeLogFile(void);

  std::string getTimeStamp(void) const;
  static Logger &newLogInstance(void) {
    static Logger logger;
    return logger;
  }

private:
  template <typename... Args>
  void create(logLevel LogLvl, const char *LvlTitle, const char *LvlColor,
              std::ostream &consoleStream, int LineNbr, const char *SrcFile, Args... args) {
    if (LogLvl < currentLevel)
      return;
    std::ostringstream logEntry;
    logEntry << LvlColor << "[" << LvlTitle << "]";
    if (enabledDetail[(int)logDetail::TimeStamp])
      logEntry << "[" << getTimeStamp() << "]";
    if (enabledDetail[(int)logDetail::SourceFile])
      logEntry << "[" << SrcFile;
    if (enabledDetail[(int)logDetail::LineNumber])
      logEntry << ":" << LineNbr << "]";
    ([&] { logEntry << " " << args; }(), ...);
    if (currentOutput != logOutput::FileOnly)
      consoleStream << logEntry.str() << RESET << std::endl;
    if (currentOutput != logOutput::ConsoleOnly && logFile.is_open())
      logFile << logEntry.str() << RESET << std::endl;
  }

public:
  template <typename... Args>
  static void Log(logLevel LogLvl, const char *LvlTitle, const char *LvlColor,
                  std::ostream &ConsoleStream, int LineNbr, const char *SrcFile, Args... args) {
    newLogInstance().create(LogLvl, LvlTitle, LvlColor, ConsoleStream, LineNbr, SrcFile, args...);
  }
};

#define LOG_TRACE(...) (Logger::Log(logLevel::Trace, "TRACE", CYAN, std::cout, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_DEBUG(...) (Logger::Log(logLevel::Debug, "DEBUG", GREEN, std::cout, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_INFO(...) (Logger::Log(logLevel::Info, "INFO", BLUE, std::cout, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_WARN(...) (Logger::Log(logLevel::Warn, "WARNING", YELLOW, std::cout, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_ERROR(...) (Logger::Log(logLevel::Error, "ERROR", RED, std::cerr, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_FATAL(...) (Logger::Log(logLevel::Fatal, "FATAL", RED, std::cerr, __LINE__, __FILE__, __VA_ARGS__))
