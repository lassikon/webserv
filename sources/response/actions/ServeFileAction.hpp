#pragma once

#include <IServeAction.hpp>
#include <Logger.hpp>

class Client;
class ServeFileAction : public IServeAction {
 private:
  bool isIndexOrDefaultFile(Client& client);
  bool isExist(std::string path) const ;
  bool isPerm(std::string path) const;
  void ifCookie(Client& client, std::string& path);
  bool isCookieFound(Client& client, std::string& path);

 public:
  ServeFileAction() = default;
  virtual ~ServeFileAction() = default;
  void execute(Client& client) override;
};
