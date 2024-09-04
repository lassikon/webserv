#pragma once

#include <Global.hpp>
#include <Logger.hpp>

#include <cerrno>
#include <cstring>
#include <exception>
#include <sstream>

#define ERR_MSG_USAGE "Usage: ./webserv OR ./webserv ./<path>/<config>"
#define ERR_MSG_NOSERVER "No valid server found in:"

enum class Error { NoError, Args, Config, Server, Socket, Cgi, Signal = 128 };

class Exception : public std::exception {
 public:
  // Template to create new try-catch block, can create multiple blocks inside each other
  // Requires: refrence to class method, reference to class instance and method arguments
  // If you have instance of MyClass as 'example' with public method 'void fn(int number)'
  // Note: If called inside class, reference to class should be 'this'
  // Example: Exception::tryCatch(&MyClass::fn, &example, 42);
  template <typename Func, typename Cref, typename... Args>
  static auto tryCatch(Func fn, Cref ref, Args&&... args) {
    std::ostringstream exceptionEntry;
    try {
      return (ref->*fn)(std::forward<Args>(args)...);
    } catch (const std::logic_error& e) {
      exceptionEntry << "Logic Error: " << e.what();
    } catch (const std::runtime_error& e) {
      exceptionEntry << "Runtime Error: " << e.what();
    } catch (const std::bad_alloc& e) {
      exceptionEntry << "Memory Error: " << e.what();
    } catch (const std::bad_exception& e) {
      exceptionEntry << "Unexpected Error: " << e.what();
    } catch (const std::exception& e) {
      // Suppresses extra logging for manual throw
      // where logging is already done by THROW
      if ((std::string)e.what() != "std::exception")
        exceptionEntry << "Exception occured: " << e.what();
    }
    if (!exceptionEntry.str().empty())
      LOG_FATAL(exceptionEntry.str());
  }

  static inline std::string expandErrno(void) noexcept {
    std::ostringstream oss;
    if (!errno) {
      oss << "";
    } else {
      oss << "[errno:" << errno << "] " << strerror(errno);
    }
    return oss.str();
  }
};

#define STRERROR Exception::expandErrno()

#define THROW(errCode, ...)         \
  LOG_FATAL(__VA_ARGS__, STRERROR); \
  g_ExitStatus = (int)errCode;      \
  throw Exception();
