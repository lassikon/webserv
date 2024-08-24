#pragma once

#include <exception>
#include <utility>

#include <Logger.hpp>

#define ERR_MSG_USAGE "Usage: ./webserv OR ./webserv ./<path>/<config>"
#define ERR_MSG_NOSERV "Could not load any server from config file"
#define ERR_MSG_CONFIG "Could not access config"

enum class ErrorCode { NoError, ArgCount = 134, ConfigFile, NoServer };

class Exception : public std::exception {
public:
  Exception(void);
  Exception(const Exception &other) = delete;
  Exception &operator=(const Exception &other) = delete;
  ~Exception(void);

private:
  std::string errCodeToString(const ErrorCode &e) noexcept;
  static Exception &newTryCatch(void) {
    static Exception wrapper;
    return wrapper;
  }

private:
  template <typename Func, typename Cref, typename... Args>
  auto wrap(Func fn, Cref ref, Args &&...args) {
    try {
      return (ref->*fn)(std::forward<Args>(args)...);
    } catch (...) {
    }
  }

public:
  template <typename Func, typename Cref, typename... Args>
  static auto tryCatch(Func fn, Cref ref, Args &&...args) {
    newTryCatch().wrap(fn, ref, args...);
  }
};

#define THROW_WARN(errMsg)                                                     \
  LOG_WARN(errMsg);                                                            \
  throw Exception();

#define THROW_ERROR(errMsg)                                                    \
  LOG_ERROR(errMsg);                                                           \
  throw Exception();

#define THROW_FATAL(errMsg)                                                    \
  LOG_FATAL(errMsg);                                                           \
  throw Exception();
