#pragma once

#include <Utility.hpp>

#include <fstream>

class SessionManager {
 private:
  const std::ifstream sessionIds;

 public:
  SessionManager(void);
  ~SessionManager(void);
};
