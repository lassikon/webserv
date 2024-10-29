#pragma once

#include <Colors.hpp>

#include <array>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

enum class logLevel { Trace, Debug, Info, Warn, Error, Fatal };
enum class logOutput { ConsoleOnly, FileOnly, Both };
enum class logDetail { Time, File, Func, Line };

class Logger {
 private:
  static std::unordered_map<std::string, bool> classFilter;

 private:
  static const char* fileName;
  static std::ofstream logFile;
  static logOutput currentOutput;
  static logLevel currentLevel;
  static std::array<bool, 4> enabledDetail;

 public:
  virtual ~Logger(void) noexcept {}

  static void loadDefaults(void);
  static void setLogOutput(const logOutput& newOutput);
  static void setLogLevel(const logLevel& newLevel);
  static void setLogDetails(bool time, bool file, bool func, bool line);
  static void setLogDetail(logDetail index, bool value);

 private:
  static void createLogFile(void) noexcept;
  static void closeLogFile(void) noexcept;

  static void insertLogDetails(std::ostringstream& log, std::string src, const char* fn, int line);
  static void printLogEntry(std::ostream& console, std::ostringstream& logEntry);
  static std::string getDateTimeStamp(void);
  static std::string filterClassName(std::string& fileName);
  static bool isFiltered(std::string& fileName) noexcept;

 public:
  // template function to expand log arguments to given ostringstream using lambda
  template <typename... Args> static void expandLogArgs(std::ostringstream& log, Args&&... args) {
    if (!sizeof...(Args)) {
      return;
    } else {
      ([&] { log << " " << args; }(), ...);
    }
  }

 public:
  // template function called by macros defined at the bottom, recieves all passed arguments
  // checks logging level and file (class) name for filtering
  // can log entries into separate file if enabled
  template <typename... Args>
  static void Log(logLevel level, const char* title, const char* color, std::ostream& console,
                  std::string fileName, const char* funcName, int lineNbr, Args&&... args) {
    if (level < currentLevel || (level < logLevel::Warn && isFiltered(fileName))) {
      return;
    } else {
      std::ostringstream logEntry;
      logEntry << color << "[" << title << "]";
      insertLogDetails(logEntry, fileName, funcName, lineNbr);
      expandLogArgs(logEntry, std::forward<Args>(args)...);
      printLogEntry(console, logEntry);
    }
  }
};

// helper macro to expand details of logging data
#define LOGDATA __FILE__, __func__, __LINE__

// defined macros to call each level of logging, does not need to be instantiated
// takes primitive variadic arguments (string, int, ...)
// does not support complex data types like std::vector or std::map
// example: LOG_ERROR("Failed to listen on socket fd:", sockFd);
#define LOG_TRACE(...) \
  (Logger::Log(logLevel::Trace, "TRACE", BLUE, std::cout, LOGDATA, __VA_ARGS__))
#define LOG_DEBUG(...) \
  (Logger::Log(logLevel::Debug, "DEBUG", GREEN, std::cout, LOGDATA, __VA_ARGS__))
#define LOG_INFO(...) (Logger::Log(logLevel::Info, "INFO", CYAN, std::cout, LOGDATA, __VA_ARGS__))
#define LOG_WARN(...) \
  (Logger::Log(logLevel::Warn, "WARNING", YELLOW, std::cout, LOGDATA, __VA_ARGS__))
#define LOG_ERROR(...) (Logger::Log(logLevel::Error, "ERROR", RED, std::cerr, LOGDATA, __VA_ARGS__))
#define LOG_FATAL(...) (Logger::Log(logLevel::Fatal, "FATAL", RED, std::cerr, LOGDATA, __VA_ARGS__))
