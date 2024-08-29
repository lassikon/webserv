#pragma once

#include <Global.hpp>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>

class Utility {
public:
  static std::string trimWhitespaces(std::string &line);
  static std::string trimComments(std::string &line);
  static std::string getDateTimeStamp(void);
  static int statusOk(void) noexcept;
};
