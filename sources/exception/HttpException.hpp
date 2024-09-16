#pragma once

#include <Response.hpp>
#include <exception>
#include <Utility.hpp>
#include <Logger.hpp>
#include <filesystem>
#include <string>
#include <vector>
#include <sys/stat.h>

class HttpException : public std::exception {
 public:
  HttpException(void) = delete;

  HttpException(Response& res, int errorCode, std::string message)
      :res(res), errorCode(errorCode), message(message) {}
  virtual ~HttpException(void) noexcept {}
  virtual const char* what() const noexcept { return message.c_str(); }
  int geterrorCode(void) const { return errorCode; }
  std::string getMessage(void) const { return message; }
  void setResponseAttributes(void);

 private:
  Response& res;
  int errorCode;
  std::string message;

  bool HttpException::isValid(std::string path) const;
};