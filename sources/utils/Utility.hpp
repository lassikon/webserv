#pragma once

#include <Global.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <typeinfo>
#include <vector>

class Utility {
 public:
  static std::string trimWhitespaces(std::string& line);
  static std::string trimComments(std::string& line);
  static std::vector<char> readFile(std::string& path);
  static std::filesystem::path getExePath(std::filesystem::path& path);
  static std::string getMimeType(std::string& extension);
  static bool statusOk(void) noexcept;

  template <class T> static std::string getConstructor(const T& object) {
    return getClassName(object) + " constructor called";
  };

  template <class T> static std::string getDeconstructor(const T& object) {
    return getClassName(object) + " deconstructor called";
  };

  template <class T> static std::string getClassName(const T& object) {
    int start_pos = 0;
    std::string str = typeid(object).name();
    for (int i = 0; !std::isdigit(str[i]); i++) {
      start_pos++;
    }
    for (int j = start_pos; std::isdigit(str[j]); j++) {
      start_pos++;
    }
    return str.substr(start_pos, str.length());
  };
};
