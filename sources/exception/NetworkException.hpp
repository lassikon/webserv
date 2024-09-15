#pragma once

#include <IException.hpp>
#include <Response.hpp>

class NetworkException : public IException {
 public:
  NetworkException() = delete;

  template <typename... Args>
  NetworkException(Response& respond, NetworkError errCode, const char* fileName,
                   const char* funcName, const size_t line, Args&&... args)
      : IException(errCode, fileName, funcName, line, std::forward<Args>(args)...) {
    setResponseAttributes(respond, (int)errCode, "Forbidden");
  }

  // Template to create new try-catch block, can create multiple blocks inside each other
  // Requires: refrence to class method, reference to class instance and method arguments
  // If you have instance of MyClass as 'example' with public method 'void fn(int number)'
  // Note: If called inside class, reference to class should be 'this'
  // Example: NetworkException::tryCatch(&MyClass::fn, &example, 42);
  template <typename Func, typename Cref, typename... Args>
  static auto tryCatch(Func fn, Cref ref, Args&&... args) {
    try {
      return (ref->*fn)(std::forward<Args>(args)...);
    } catch (const NetworkException& e) {
      std::cerr << e.what();
    } catch (const std::logic_error& e) {
      LOG_FATAL("[STD] Logic Error: ", e.what());
    } catch (const std::runtime_error& e) {
      LOG_FATAL("[STD] Runtime Error: ", e.what());
    } catch (const std::bad_alloc& e) {
      LOG_FATAL("[STD] Memory Error: ", e.what());
    } catch (const std::bad_exception& e) {
      LOG_FATAL("[STD] Unexpected Error: ", e.what());
    } catch (const std::exception& e) {
      LOG_FATAL("[STD] Skill issues: ", e.what());
    }
  }

 private:
  void setResponseAttributes(Response& respond, int errorCode, const char* message) {
    respond.setResStatusCode(errorCode);
    respond.setResStatusMessage(message);
    auto key = respond.getServerConfig().pagesCustom.find(errorCode);
    if (key == respond.getServerConfig().pagesCustom.end()) {
      key = respond.getServerConfig().pagesDefault.find(errorCode);
    }
    std::string path = key->second;
    std::filesystem::path exePath;
    exePath = Utility::getExePath(exePath);
    if (path.front() == '/') {
      path = path.substr(1, path.size());
    }
    std::filesystem::path errorPath = exePath / path;
    std::string errorPathStr = errorPath.string();
    std::vector<char> ibody = Utility::readFile(errorPathStr);
    respond.setResBody(ibody);
    std::string ext = errorPathStr.substr(errorPathStr.find_last_of(".") + 1);
    std::string mimeType = Utility::getMimeType(ext);
    respond.addHeader("Cache-Control", "max-age=3600, must-revalidate");
    respond.addHeader("Content-Type", mimeType);
    respond.addHeader("Content-Length", std::to_string(ibody.size()));
    respond.addHeader("Connection", "close");
  }
};

#define httpForbidden(response, ...) \
  NetworkException(response, NetworkError::Forbidden, DATA, __VA_ARGS__)
