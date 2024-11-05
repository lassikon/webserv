#pragma once

#include <cerrno>
#include <cstring>
#include <exception>
#include <sstream>
#include <string>
#include <utility>

#include <Colors.hpp>
#include <Global.hpp>
#include <Logger.hpp>

#define ERR_MSG_USAGE "Usage: ./webserv OR ./webserv <path>/<config>"
#define ERR_MSG_CONFIG "No valid server found in:"
#define ERR_MSG_SERVER "Could not setup any servers from:"
#define ERR_MSG_NOFILE "Could not access file:"
#define ERR_MSG_EMPTYFILE "Config file is empty:"
#define ERR_MSG_SIGNAL "Server interrupted by signal:"

enum class RuntimeError {
  NoError = 0,
  Args = 1,
  Config = 2,
  Server = 3,
  Client = 4,
  Socket = 5,
  Cgi = 6,
  Signal = 128
};

enum class NetworkError {
  NoError = 200,
  BadRequest = 400,
  Forbidden = 403,
  Notfound = 404,
  Method = 405,
  Length = 411,
  Payload = 413,
  Internal = 500,
  BadGateway = 502,
  GatewayTimeout = 504,
  Version = 505
};

// interface for exceptios, inherits std::exception and logger
class IException : public std::exception, public Logger {
 private:
  std::string logEntry;
  const char* file;
  const char* func;
  int line;

 public:
  virtual ~IException(void) noexcept {}

  // override function for custom throw and e.what()
  virtual const char* what() const noexcept { return logEntry.c_str(); }

 public:
  // template function for network exception macros
  // file, func, and line are passed with logger's LOGDATA
  // get NetworkError error code for possible post prosessing
  // takes variadic arguments for logging purposes but
  // does not support complex data types like std::vector
  // updates logEntry which is used by e.what() override
  template <typename... Args>
  IException(NetworkError err, const char* file, const char* func, int line, Args&&... args)
      : file(file), func(func), line(line) {
    logEntry = createLogEntry(logLevel::Error, "ERROR", "Network", std::forward<Args>(args)...);
    (void)err;
  };

 public:
  // template function for runtime exception macros
  // file, func, and line are passed with logger's LOGDATA
  // gets RuntimeError for possible post prosessing
  // takes variadic arguments for logging purposes but
  // does not support complex data types like std::vector
  // updates logEntry which is used by e.what() override
  // was changed from fatal to non-fatal during development
  template <typename... Args>
  IException(RuntimeError err, const char* file, const char* func, int line, Args&&... args)
      : file(file), func(func), line(line) {
    logEntry = createLogEntry(logLevel::Error, "ERROR", "Runtime", std::forward<Args>(args)...);
    // g_ExitStatus = (int)err;
    (void)err;
    g_ExitStatus = 0;
  };

 private:
  // private template function used by public templates
  // needs logLevel, title and error type from public template
  // the rest of the arguments are obtained throug macro call
  // uses logger to generate log entry into console and file
  template <typename... Args>
  std::string createLogEntry(logLevel lvl, const char* title, const char* errType, Args&&... args) {
    std::ostringstream oss;
    oss << errType << " error:";
    expandLogArgs(oss, std::forward<Args>(args)...);
    if (lvl == logLevel::Fatal) {
      oss << expandErrno();
    }
    std::ostringstream log;
    Log(lvl, title, BRIGHTRED, log, file, func, line, oss.str());
    return log.str();
  }

 public:
  // function to expand errno, does not need to be instantiated
  static std::string expandErrno(void) noexcept {
    if (!errno) {
      return std::string();
    }
    std::ostringstream oss;
    oss << " [" << errno << "] " << strerror(errno);
    return oss.str();
  }
};
