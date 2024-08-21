#include <Logger.hpp>

Logger::Logger(void)
    : currentLevel(logLevel::Trace), currentOutput(logOutput::Both) {
  if (currentOutput != logOutput::ConsoleOnly)
    createLogFile();
}

Logger::~Logger(void) { closeLogFile(); }

void Logger::createLogFile(void) {
  logFile.open(LOGFILE, std::fstream::out);
  if (logFile.fail())
    std::cerr << LOGFILE << ": " << strerror(errno) << std::endl;
}

void Logger::closeLogFile(void) {
  if (logFile.is_open())
    logFile.close();
}

void Logger::setLogLevel(logLevel newLevel) { currentLevel = newLevel; }
void Logger::setLogOutput(logOutput newOutput) { currentOutput = newOutput; }

std::string Logger::levelToString(logLevel Level) const {
  switch (Level) {
  case logLevel::Trace:
    return "TRACE";
  case logLevel::Debug:
    return "DEBUG";
  case logLevel::Info:
    return "INFO";
  case logLevel::Warn:
    return "WARNING";
  case logLevel::Error:
    return "ERROR";
  case logLevel::Critical:
    return "CRITICAL";
  default:
    return "UNKNOWN";
  }
}

std::string Logger::logTimeStamp(void) const {
  time_t now = time(0);
  tm *timeInfo = localtime(&now);
  char timeStamp[20];
  strftime(timeStamp, sizeof(timeStamp), "%Y-%m-%d %H:%M:%S", timeInfo);
  return timeStamp;
}
