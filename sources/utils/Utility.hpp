#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

class Utility {
public:
  Utility(void){};
  Utility(const Utility &) = delete;
  Utility &operator=(const Utility &) = delete;
  ~Utility(void){};

  static std::string trimCommentsAndWhitespaces(std::string &line);
  static std::string getDateTimeStamp(void);
};
