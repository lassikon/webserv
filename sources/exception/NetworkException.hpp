#pragma once

#include <Client.hpp>
#include <IException.hpp>
#include <Response.hpp>
#include <string>
#include <unordered_map>

class NetworkException : public IException {
 private:
  std::unordered_map<NetworkError, std::string> errMsgMap = {
    {NetworkError::BadRequest, "Bad Request"},    {NetworkError::Forbidden, "Forbidden"},
    {NetworkError::Notfound, "Not Found"},        {NetworkError::Method, "Method Not Allowed"},
    {NetworkError::Payload, "Payload Too Large"},
  };

 public:
  NetworkException() = delete;

  template <typename... Args>
  NetworkException(Client& client, NetworkError errCode, const char* fileName, const char* funcName,
                   const size_t line, Args&&... args)
      : IException(errCode, fileName, funcName, line, std::forward<Args>(args)...) {
    setResponseAttributes(client.getRes(), (int)errCode, getErrorMessage(errCode));
    client.setClientState(ClientState::SENDING);
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
  const char* getErrorMessage(NetworkError errCode) {
    for (const auto& [code, message] : errMsgMap) {
      if (code == errCode) {
        return message.c_str();
      }
    }
    return "Unknown Error";
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
    if (!std::filesystem::exists(errorPath) || !isValid(errorPathStr)) {
      setBasicErrorPage(respond);
      return;
    }
    std::vector<char> ibody = Utility::readFile(errorPathStr);
    respond.setResBody(ibody);
    std::string ext = errorPathStr.substr(errorPathStr.find_last_of(".") + 1);
    std::string mimeType = Utility::getMimeType(ext);
    respond.addHeader("Cache-Control", "max-age=3600, must-revalidate");
    respond.addHeader("Content-Type", mimeType);
    respond.addHeader("Content-Length", std::to_string(ibody.size()));
    respond.addHeader("Connection", "close");
  }

  bool isValid(std::string path) const {
    struct stat s;
    if (!stat(path.c_str(), &s) && S_ISREG(s.st_mode) && !access(path.c_str(), X_OK)) {
      return true;
    }
    return false;
  }

  void setBasicErrorPage(Response& res) {
    std::string body("<html><body><h1>404 Not Found, error page not found!</h1></body></html>");
    std::vector<char> ibody(body.begin(), body.end());
    res.setResStatusCode(404);
    res.setResStatusMessage("Not Found");
    res.setResBody(ibody);
    res.addHeader("Content-Type", "text/html");
    res.addHeader("Connection", "keep-alive");
    res.addHeader("Content-Length", std::to_string(ibody.size()));
    return;
  }
};

#define httpForbidden(client, ...) \
  NetworkException(client, NetworkError::Forbidden, LOGDATA, __VA_ARGS__)
#define httpNotFound(client, ...) \
  NetworkException(client, NetworkError::Notfound, LOGDATA, __VA_ARGS__)
#define httpBadRequest(client, ...) \
  NetworkException(client, NetworkError::BadRequest, LOGDATA, __VA_ARGS__)
#define httpMethod(client, ...) NetworkException(client, NetworkError::Method, LOGDATA, __VA_ARGS__)
#define httpPayload(client, ...) \
  NetworkException(client, NetworkError::Payload, LOGDATA, __VA_ARGS__)
