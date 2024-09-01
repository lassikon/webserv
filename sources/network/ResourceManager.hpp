#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <memory>

class ResourceManager {
 private:
  std::map<std::string, std::unique_ptr<std::fstream>> fileStreams;

 public:
  ResourceManager(void);
  ~ResourceManager(void);
  std::fstream& openFile(const std::string& path);
  void closeFile(const std::string& path);
  void closeAllFileStreams(void);
};