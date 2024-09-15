#pragma once

#include <map>
#include <sstream>
#include <string>

class UrlEncoder {
 private:
  static bool isUnreserved(char c);
  static std::string charToHex(char c);
  static char hexToChar(const std::string& hex);

 public:
  static std::string encode(const std::string& str);
  static std::string decode(const std::string& str);
};
