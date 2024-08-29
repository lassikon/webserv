#include <Utility.hpp>

std::string Utility::trimWhitespaces(std::string &line) {
  size_t pos = line.find_first_not_of(" \t");
  if (pos != std::string::npos)
    line = line.substr(pos);
  pos = line.find_last_not_of(" \t");
  if (pos != std::string::npos)
    line = line.substr(0, pos + 1);
  return line;
}

std::string Utility::trimComments(std::string &line) {
  size_t pos = line.find("#");
  if (pos != std::string::npos)
    line = line.substr(0, pos);
  return line;
}

std::string Utility::getDateTimeStamp(void) {
  auto now = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(now);
  std::ostringstream ss;
  ss << std::put_time(std::localtime(&tt), "%Y-%m-%d %X");
  return ss.str();
}

int Utility::statusOk(void) noexcept { return !g_ExitStatus; }
