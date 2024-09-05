#pragma once
#include <Config.hpp>
#include <IDirectiveSetter.hpp>
#include <Logger.hpp>
#include <string>
#include <filesystem>
#include <regex>

class RouteDirectiveSetter : public IDirectiveSetter {
 public:
  RouteDirectiveSetter() = default;
  virtual ~RouteDirectiveSetter() = default;
  void handleDirective(void* data, std::string& key, std::string& value,
                       int& lineNumber) override;
  void setLocation(RouteConfig& route, std::string& value, int& lineNumber);
  void setMethods(RouteConfig& route, std::string& value, int& lineNumber);
  void setRoot(RouteConfig& route, std::string& value, int& lineNumber);
  void setDirectoryListing(RouteConfig& route, std::string& value,
                           int& lineNumber);
  void setDefaultFile(RouteConfig& route, std::string& value, int& lineNumber);
  void setUploadPath(RouteConfig& route, std::string& value, int& lineNumber);
  void setRedirect(RouteConfig& route, std::string& value, int& lineNumber);
  void setCgi(RouteConfig& route, std::string& value, int& lineNumber);
};