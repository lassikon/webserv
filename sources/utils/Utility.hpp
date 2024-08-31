#pragma once

#include <Logger.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>

class Utility {
 public:
  static std::string trimWhitespaces(std::string& line);
  static std::string trimComments(std::string& line);
  static std::vector<char> readFile(std::string& path);
  static std::filesystem::path getExePath(std::filesystem::path& path);
  static std::string getMimeType(std::string& extension);
};