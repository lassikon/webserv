#include <Logger.hpp>

Logger::Logger(void) {
  currentLevel = logLevel::Trace;
  currentOutput = logOutput::Both;
  enabledDetail[(int)logDetail::TimeStamp] = false;
  enabledDetail[(int)logDetail::SourceFile] = true;
  enabledDetail[(int)logDetail::LineNumber] = true;
  if (currentOutput != logOutput::ConsoleOnly)
    createLogFile();
}

Logger::~Logger(void) { closeLogFile(); }

void Logger::createLogFile(void) {
  logFile.open(fileName, std::fstream::out);
  if (logFile.fail())
    std::cerr << fileName << ": " << strerror(errno) << std::endl;
}

void Logger::closeLogFile(void) {
  if (logFile.is_open())
    logFile.close();
}

std::string Logger::getTimeStamp(void) const {
  time_t now = time(0);
  tm *timeInfo = localtime(&now);
  char timeStamp[20];
  strftime(timeStamp, sizeof(timeStamp),
    "%Y-%m-%d %H:%M:%S", timeInfo);
  return timeStamp;
}
