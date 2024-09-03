#pragma once

#include <Global.hpp>
#include <Logger.hpp>

#include <cerrno>
#include <cstring>
#include <exception>
#include <sstream>

#define ERR_MSG_USAGE "Usage: ./webserv OR ./webserv ./<path>/<config>"
#define ERR_MSG_NOSERVER "No valid server found in:"
#define ERR_MSG_NOFILE "Could not access file:"
#define ERR_MSG_SIGNAL "Server interrupted by signal:"

enum class Error { NoError, Args, Config, Server, Signal = 128 };

class Exception : public std::exception {

private:
  static inline Exception &newTryCatch(void) noexcept {
    static Exception wrapper;
    return wrapper;
  }

private:
  template <typename Func, typename Cref, typename... Args> auto create(Func fn, Cref ref, Args &&...args) {
    std::ostringstream ss;
    try {
      return (ref->*fn)(std::forward<Args>(args)...);
    } catch (const std::logic_error &e) {
      ss << "Logic Error: " << e.what();
    } catch (const std::runtime_error &e) {
      ss << "Runtime Error: " << e.what();
    } catch (const std::bad_alloc &e) {
      ss << "Memory Error: " << e.what();
    } catch (const std::bad_exception &e) {
      ss << "Unexpected Error: " << e.what();
    } catch (const std::exception &e) {
      if ((std::string)e.what() != "std::exception")
        ss << "Exception occured: " << e.what();
    }
    if (!ss.str().empty())
      LOG_FATAL(ss.str());
  }

public:
  template <typename Func, typename Cref, typename... Args> static auto tryCatch(Func fn, Cref ref, Args &&...args) {
    newTryCatch().create(fn, ref, args...);
  }

#define STRERROR Exception::expandErrno()
  static inline std::string expandErrno(void) noexcept {
    std::ostringstream ss;
    if (!errno) {
      ss << "";
    } else {
      ss << "[errno:" << errno << "] " << strerror(errno);
    }
    return ss.str();
  }
};

#define THROW(errCode, ...)                                                                                            \
  LOG_FATAL(__VA_ARGS__);                                                                                              \
  g_ExitStatus = (int)errCode;                                                                                         \
  throw Exception();
