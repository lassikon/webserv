#include <Exception.hpp>
#include <Logger.hpp>

Logger::Logger(void) {
  loadDefaults();
}

Logger::~Logger(void) {
  closeLogFile();
}

void Logger::loadDefaults(void) {
  currentLevel = logLevel::Trace;
  currentOutput = logOutput::Both;
  setLogDetails(true, true, true, true);
  if (currentOutput != logOutput::ConsoleOnly)
    createLogFile();
}

void Logger::createLogFile(void) {
  logFile.open(fileName, std::ios_base::app);
  if (logFile.fail())
    LOG_WARN(ERR_MSG_NOFILE, fileName, STRERROR);
}

void Logger::closeLogFile(void) {
  if (logFile.is_open())
    logFile.close();
}

void Logger::setLogDetails(bool time, bool file, bool func, bool line) {
  setLogDetail((int)logDetail::Time, time);
  setLogDetail((int)logDetail::File, file);
  setLogDetail((int)logDetail::Func, func);
  setLogDetail((int)logDetail::Line, line);
}

void Logger::setLogDetail(int index, bool value) {
  enabledDetail[index] = value;
}

std::string Logger::getDateTimeStamp(void) {
  auto now = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(now);
  std::ostringstream ss;
  ss << std::put_time(std::localtime(&tt), "%Y-%m-%d %X");
  return ss.str();
}
