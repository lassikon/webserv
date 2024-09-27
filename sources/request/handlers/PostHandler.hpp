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
  void processMultipartFormData(Client& client);
  std::string extractBoundary(Client& client);
  std::vector<std::string> splitByBoundary(std::string data, std::string boundary);
  bool isFilePart(const std::string& part);
  std::string extractFileName(const std::string& part);
  std::string extractFileData(const std::string& part);
  void processFilePart(Client& client, const std::string& part);
  void processFormData(const std::string& part);

  void setResponse(Client& client);
};