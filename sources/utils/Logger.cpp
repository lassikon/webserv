#include <Exception.hpp>
#include <Logger.hpp>

Logger::Logger(void) {
  currentLevel = logLevel::Trace;
  currentOutput = logOutput::Both;
  enabledDetail[(int)logDetail::TimeStamp] = true;
  enabledDetail[(int)logDetail::SourceFile] = true;
  enabledDetail[(int)logDetail::LineNumber] = true;
  if (currentOutput != logOutput::ConsoleOnly)
    createLogFile();
}

Logger::~Logger(void) { closeLogFile(); }

void Logger::createLogFile(void) {
  logFile.open(fileName, std::ios_base::app);
  if (logFile.fail())
    LOG_WARN(ERR_MSG_NOCONFIG, fileName, ":", strerror(errno));
}

void Logger::closeLogFile(void) {
  if (logFile.is_open())
    logFile.close();
}

std::string Logger::getTimeStamp(void) const {
  auto now = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(now);
  std::ostringstream ss;
  ss << std::put_time(std::localtime(&tt), "%Y-%m-%d %X");
  return ss.str();
}
