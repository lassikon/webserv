#pragma once

#include <IRequestHandler.hpp>

class Client;

class PostHandler : public IRequestHandler {
 private:
  std::string contentType;
  std::unordered_map<std::string, std::string> formData;

 public:
  PostHandler() = default;
  virtual ~PostHandler() = default;
  void executeRequest(Client& client) override;

 private:
  void getContentType(Client& client);
  void processFormUrlEncoded(Client& client);

  void setResponse(Client& client);
};