#pragma once

#include <Colors.hpp>
#include <Utility.hpp>

#include <fstream>
#include <iostream>
#include <sstream>

enum class logLevel { Trace, Debug, Info, Warn, Error, Fatal };

class Logger {
private:
  enum class logOutput { ConsoleOnly, FileOnly, Both };
  enum class logDetail { Time, File, Line };
  const char *fileName = "webserv.log";
  std::ofstream logFile;
  logOutput currentOutput;
  logLevel currentLevel;
  bool enabledDetail[3];

public:
  Logger(void);
  ~Logger(void);

private:
  void loadDefaults(void);
  void createLogFile(void);
  void closeLogFile(void);
  void setLogDetails(bool time, bool file, bool line);
  void setLogDetail(int index, bool value);

private:
  static inline Logger &newLogInstance(void) noexcept {
    static Logger logger;
    return logger;
  }

private:
  template <typename... Args>
  void create(logLevel LogLvl, const char *LvlTitle, const char *LvlColor,
              std::ostream &consoleStream, int LineNbr, const char *SrcFile, Args &&... args) {
    if (LogLvl < currentLevel)
      return;
    std::ostringstream logEntry;
    logEntry << LvlColor << "[" << LvlTitle << "]";
    if (enabledDetail[(int)logDetail::Time])
      logEntry << "[" << Utility::getDateTimeStamp() << "]";
    if (enabledDetail[(int)logDetail::File])
      logEntry << "[" << SrcFile;
    if (enabledDetail[(int)logDetail::Line])
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
                  std::ostream &ConsoleStream, int LineNbr, const char *SrcFile, Args &&... args) {
    newLogInstance().create(LogLvl, LvlTitle, LvlColor, ConsoleStream, LineNbr, SrcFile, args...);
  }
};

#define LOG_TRACE(...) (Logger::Log(logLevel::Trace, "TRACE", CYAN, std::cout, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_DEBUG(...) (Logger::Log(logLevel::Debug, "DEBUG", GREEN, std::cout, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_INFO(...) (Logger::Log(logLevel::Info, "INFO", BLUE, std::cout, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_WARN(...) (Logger::Log(logLevel::Warn, "WARNING", YELLOW, std::cout, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_ERROR(...) (Logger::Log(logLevel::Error, "ERROR", RED, std::cerr, __LINE__, __FILE__, __VA_ARGS__))
#define LOG_FATAL(...) (Logger::Log(logLevel::Fatal, "FATAL", RED, std::cerr, __LINE__, __FILE__, __VA_ARGS__))
