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
#include <typeinfo>
#include <vector>

class Utility {
public:
  static std::string trimWhitespaces(std::string &line);
  static std::string trimComments(std::string &line);
  static std::vector<char> readFile(std::string &path);
  static std::filesystem::path getExePath(std::filesystem::path &path);
  static std::string getMimeType(std::string &extension);
  static std::string getDateTimeStamp(void);
  static int statusOk(void) noexcept;

  template <typename T> static std::string getConstructor(T &object) {
    int start_pos = 0;
    std::string str = typeid(object).name();
    for (int i = 0; !std::isdigit(str[i]); i++) {
      start_pos++;
    }
    for (int j = start_pos; std::isdigit(str[j]); j++) {
      start_pos++;
    }
    return (str.substr(start_pos, str.length()) + "constructor called");
  };

  template <typename T> static std::string getDeconstructor(T &object) {
    int start_pos = 0;
    std::string str = typeid(object).name();
    for (int i = 0; !std::isdigit(str[i]); i++) {
      start_pos++;
    }
    for (int j = start_pos; std::isdigit(str[j]); j++) {
      start_pos++;
    }
    return (str.substr(start_pos, str.length()) + "deconstructor called");
  };
};
