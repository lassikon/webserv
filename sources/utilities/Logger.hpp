#pragma once

#include <Colors.hpp>

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

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

  void setLogDetails(bool time, bool file, bool func, bool line);
  void setLogDetail(int index, bool value);
  std::string getDateTimeStamp(void) const;

  void insertLogDetails(std::ostringstream& log, std::string src, const char* fn, int line);
  void printLogEntry(std::ostream& console, std::ostringstream& logEntry);

 private:
  template <typename... Args>
  void create(logLevel level, const char* title, const char* color, std::ostream& console,
              std::string source, const char* function, int line, Args&&... args) {
    if (level < currentLevel) {
      return;
    } else {
      std::ostringstream logEntry;
      logEntry << color << "[" << title << "]";
      insertLogDetails(logEntry, source, function, line);
      ([&] { logEntry << " " << args; }(), ...);
      printLogEntry(console, logEntry);
    }
  }

 private:
  static inline Logger& newLogInstance(void) noexcept {
    static Logger logger;
    return logger;
  }

 public:
  template <typename... Args>
  static void Log(logLevel level, const char* title, const char* color, std::ostream& console,
                  std::string source, const char* function, int line, Args&&... args) {
    newLogInstance().create(level, title, color, console, source, function, line, args...);
  }
};

#define LOG_TRACE(...) (Logger::Log(logLevel::Trace, "TRACE", CYAN, std::cout, __FILE__, __func__, __LINE__, __VA_ARGS__))
#define LOG_DEBUG(...) (Logger::Log(logLevel::Debug, "DEBUG", GREEN, std::cout, __FILE__, __func__, __LINE__, __VA_ARGS__))
#define LOG_INFO(...) (Logger::Log(logLevel::Info, "INFO", BLUE, std::cout, __FILE__, __func__, __LINE__, __VA_ARGS__))
#define LOG_WARN(...) (Logger::Log(logLevel::Warn, "WARNING", YELLOW, std::cout, __FILE__, __func__, __LINE__, __VA_ARGS__))
#define LOG_ERROR(...) (Logger::Log(logLevel::Error, "ERROR", RED, std::cerr, __FILE__, __func__, __LINE__, __VA_ARGS__))
#define LOG_FATAL(...) (Logger::Log(logLevel::Fatal, "FATAL", RED, std::cerr, __FILE__, __func__, __LINE__, __VA_ARGS__))
