#pragma once

#include <Global.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

class Utility {
public:
  static std::string trimCommentsAndWhitespaces(std::string &line);
  static std::string getDateTimeStamp(void);
  static int statusOk(void) noexcept;
};
