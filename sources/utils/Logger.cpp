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
    LOG_WARN("Could not create log: ", fileName, ": ", strerror(errno));
}

void Logger::closeLogFile(void) {
  if (logFile.is_open())
    logFile.close();
}

std::string Logger::getTimeStamp(void) const {
  time_t now = time(0);
  tm *timeInfo = localtime(&now);
  char timeStamp[20];
  strftime(timeStamp, sizeof(timeStamp), "%Y-%m-%d %H:%M:%S", timeInfo);
  return timeStamp;
}
