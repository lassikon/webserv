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
#define ERR_MSG_NOSERVER "No valid server found in:"
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
  NoError = 0,
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

class IException : public std::exception, public Logger {
 private:
  std::string logEntry;
  const char* file;
  const char* func;
  int line;

 public:
  virtual ~IException(void) noexcept {}

  virtual const char* what() const noexcept { return logEntry.c_str(); }

 public:
  template <typename... Args>
  IException(NetworkError err, const char* file, const char* func, int line, Args&&... args)
      : file(file), func(func), line(line) {
    logEntry = createLogEntry(logLevel::Error, "ERROR", "Network", std::forward<Args>(args)...);
    (void)err;
  };

 public:
  template <typename... Args>
  IException(RuntimeError err, const char* file, const char* func, int line, Args&&... args)
      : file(file), func(func), line(line) {
    logEntry = createLogEntry(logLevel::Fatal, "FATAL", "Runtime", std::forward<Args>(args)...);
    // g_ExitStatus = (int)err;
    (void)err;
    g_ExitStatus = 0;
  };

 private:
  template <typename... Args>
  std::string createLogEntry(logLevel lvl, const char* title, const char* errType, Args&&... args) {
    std::ostringstream oss;
    oss << errType << "error:";
    expandLogArgs(oss, std::forward<Args>(args)...);
    if (lvl == logLevel::Fatal) {
      oss << expandErrno();
    }
    std::ostringstream log;
    Log(lvl, title, RED, log, file, func, line, oss.str());
    return log.str();
  }

 public:
  static std::string expandErrno(void) noexcept {
    if (!errno) {
      return std::string();
    }
    std::ostringstream oss;
    oss << " [" << errno << "] " << strerror(errno);
    return oss.str();
  }
};
