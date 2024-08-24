#pragma once

#include <cerrno>
#include <cstring>
#include <exception>
#include <stdexcept>
#include <string>
#include <utility>

#include <Logger.hpp>

enum class ErrorCode { ArgCount, ConfigFile, NoServer };

class Exception : public std::exception {
protected:
  const char *errMsg;
  int errCode;

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
  auto wrapper(Func fn, Cref ref, Args &&...args) {
    try {
      return (ref->*fn)(std::forward<Args>(args)...);
    } catch (const ErrorCode &e) {
      LOG_ERROR(errCodeToString(e));
    } catch (std::runtime_error &e) {
      LOG_ERROR(e.what());
    } catch (exception &e) {
      LOG_ERROR("Exception Occured: ", e.what());
    } catch (...) {
      LOG_FATAL("Unexpected Error");
    }
  }

public:
  template <typename Func, typename Cref, typename... Args>
  static auto tryCatch(Func fn, Cref ref, Args &&...args) {
    newTryCatch().wrapper(fn, ref, args...);
  }
};
