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
    setResponseAttributes(client, (int)errCode, getErrorMessage(errCode));
    client.setClientState(ClientState::PREPARING);
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
  void setResponseAttributes(Client& client, int errorCode, const char* message) {
    client.getRes().setResStatusCode(errorCode);
    client.getRes().setResStatusMessage(message);
    auto key = client.getRes().getServerConfig().pagesCustom.find(errorCode);
    if (key == client.getRes().getServerConfig().pagesCustom.end()) {
      key = client.getRes().getServerConfig().pagesDefault.find(errorCode);
    }
    std::string path = key->second;
    std::filesystem::path exePath;
    exePath = Utility::getExePath(exePath);
    if (path.front() == '/') {
      path = path.substr(1, path.size());
    }
    std::filesystem::path errorPath = exePath / path;
    std::string errorPathStr = errorPath.string();
    LOG_ERROR("Error page path:", errorPathStr);
    if (!std::filesystem::exists(errorPath) || !isValid(errorPathStr)) {
      setBasicErrorPage(client);
      return;
    }
    std::vector<char> ibody = Utility::readFile(errorPathStr);
    client.getRes().setResBody(ibody);
    std::string ext = errorPathStr.substr(errorPathStr.find_last_of(".") + 1);
    std::string mimeType = Utility::getMimeType(ext);
    client.getRes().addHeader("Cache-Control", "max-age=3600, must-revalidate");
    client.getRes().addHeader("Content-Type", mimeType);
    client.getRes().addHeader("Content-Length", std::to_string(ibody.size()));
    client.getRes().addHeader("Connection", client.getReq().getHeaders()["Connection"]);
  }

  bool isValid(std::string path) const {
    struct stat s;
    if (!stat(path.c_str(), &s) && S_ISREG(s.st_mode) && !access(path.c_str(), R_OK)) {
      return true;
    }
    return false;
  }

  void setBasicErrorPage(Client& client) {
    std::string body("<html><body><h1>404 Not Found, error page not found!</h1></body></html>");
    std::vector<char> ibody(body.begin(), body.end());
    client.getRes().setResStatusCode(404);
    client.getRes().setResStatusMessage("Not Found");
    client.getRes().setResBody(ibody);
    client.getRes().addHeader("Content-Type", "text/html");
    client.getRes().addHeader("Connection", client.getReq().getHeaders()["Connection"]);
    client.getRes().addHeader("Content-Length", std::to_string(ibody.size()));
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
