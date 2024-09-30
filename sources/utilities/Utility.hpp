#pragma once

#include <fcntl.h>
#include <CgiHandler.hpp>
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
  static size_t convertSizetoBytes(std::string& size);
  static void setNonBlocking(int& fd);
  static void setCloseOnExec(int& fd);
  static bool isCgiFd(int fd);
  static int getClientFdFromCgiParams(int fd);
  static int getOutReadFdFromClientFd(int fd);
  static int getInWriteFdFromClientFd(int fd);
  static bool getLineVectoStr(std::vector<char>& buffer ,std::string& line, size_t& curr, size_t& end);

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
