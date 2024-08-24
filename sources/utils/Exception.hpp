#pragma once

#include <exception>

#include <Logger.hpp>

enum class ServerException { ArgCount, NoFile };

class Exception : public std::exception {
public:
  Exception(void);
  Exception(const Exception &other) = delete;
  Exception &operator=(const Exception &other) = delete;
  ~Exception(void);

private:
  static Exception &newException(void) {
    static Exception handler;
    return handler;
  }

private:
  template <typename Func, typename... Args>
  auto caught(Func fn, Args &&...args) {
    try {
      return (this->*fn)(std::forward<Args>(args)...);
    } catch (ServerException &e) {
      LOG_ERROR("Server Exception: ", e);
    } catch (exception &e) {
      LOG_ERROR("Server Exception: ", e.what());
    } catch (...) {
      LOG_FATAL("Unknown Error");
    }
  }

public:
  template <typename Func, typename... Args>
  static auto safeCall(Func fn, Args &&...args) {
    newException().caught(fn, args...);
  }
};

/* #define SAFECALL(fn, ...) (ErrorHandler::safeCall(fn, &&__VA_ARGS__)) */
/* virtual const char *what() const noexcept {} */
