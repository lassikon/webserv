#include <Exception.hpp>
#include <Logger.hpp>

Logger::Logger(void) { loadDefaults(); }

Logger::~Logger(void) { closeLogFile(); }

void Logger::loadDefaults(void) {
  currentLevel = logLevel::Trace;
  currentOutput = logOutput::Both;
  setLogDetails(true, true, true);
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

void Logger::setLogDetails(bool time, bool file, bool line) {
  setLogDetail((int)logDetail::Time, time);
  setLogDetail((int)logDetail::File, file);
  setLogDetail((int)logDetail::Line, line);
}

void Logger::setLogDetail(int index, bool value) { enabledDetail[index] = value; }
