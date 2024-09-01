#pragma once

#include <Global.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

class Utility {
 public:
  static std::string trimWhitespaces(std::string& line);
  static std::string trimComments(std::string& line);
  static std::vector<char> readFile(std::string& path);
  static std::filesystem::path getExePath(std::filesystem::path& path);
  static std::string getMimeType(std::string& extension);
  static std::string getDateTimeStamp(void);
  static int statusOk(void) noexcept;
};
