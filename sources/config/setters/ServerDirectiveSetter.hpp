#pragma once

#include <Config.hpp>
#include <IDirectiveSetter.hpp>
#include <Logger.hpp>
#include <filesystem>
#include <regex>

#include <string>

class ServerDirectiveSetter : public IDirectiveSetter {
 public:
  ServerDirectiveSetter() = default;
  virtual ~ServerDirectiveSetter() = default;
  void handleDirective(void* data, std::string& key, std::string& value, int& lineNumber) override;
  void setIP(ServerConfig& server, std::string& value, int& lineNumber);
  void setServerName(ServerConfig& server, std::string& value, int& lineNumber);
  void setPort(ServerConfig& server, std::string& value, int& lineNumber);
  void setErrorPages(ServerConfig& server, std::string& value, int& lineNumber);
  void setClientBodySizeLimit(ServerConfig& server, std::string& value, int& lineNumber);
  void setCgiInterpreters(ServerConfig& server, std::string& value, int& lineNumber);
};