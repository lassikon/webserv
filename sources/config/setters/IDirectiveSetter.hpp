#pragma once
#include <string>

class IDirectiveSetter {
 public:
  virtual void handleDirective(void* data, std::string& key, std::string& value, int& lineNumber) = 0;
  virtual ~IDirectiveSetter() = default;
};