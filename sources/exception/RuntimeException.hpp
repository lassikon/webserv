#pragma once

#include <IException.hpp>

class RuntimeException : public IException {
 public:
  RuntimeException() = delete;

  template <typename... Args>
  RuntimeException(RuntimeError err, const char *file, const char *func, int line, Args &&...args)
      : IException(err, file, func, line, std::forward<Args>(args)...) {}

  // Template to create new try-catch block, can create multiple blocks inside each other
  // Requires: refrence to class method, reference to class instance and method arguments
  // If you have instance of MyClass as 'example' with public method 'void fn(int number)'
  // Note: If called inside class, reference to class should be 'this'
  // Example: RuntimeException::tryCatch(&MyClass::fn, &example, 42);
  template <typename Func, typename Cref, typename... Args>
  static void tryCatch(Func fn, Cref ref, Args &&...args) {
    try {
      return (ref->*fn)(std::forward<Args>(args)...);
    } catch (const RuntimeException &e) {
      std::cerr << e.what();
    } catch (const std::logic_error &e) {
      LOG_FATAL("[STD] Logic Error: ", e.what());
    } catch (const std::runtime_error &e) {
      LOG_FATAL("[STD] Runtime Error: ", e.what());
    } catch (const std::bad_alloc &e) {
      LOG_FATAL("[STD] Memory Error: ", e.what());
    } catch (const std::bad_exception &e) {
      LOG_FATAL("[STD] Unexpected Error: ", e.what());
    } catch (const std::exception &e) {
      LOG_FATAL("[STD] Skill issues: ", e.what());
    }
  }
};

#define configError(...) RuntimeException(RuntimeError::Config, DATA, __VA_ARGS__)
#define serverError(...) RuntimeException(RuntimeError::Server, DATA, __VA_ARGS__)
#define clientError(...) RuntimeException(RuntimeError::Client, DATA, __VA_ARGS__)
#define socketError(...) RuntimeException(RuntimeError::Socket, DATA, __VA_ARGS__)
#define cgiError(...) RuntimeException(RuntimeError::Cgi, DATA, __VA_ARGS__)
