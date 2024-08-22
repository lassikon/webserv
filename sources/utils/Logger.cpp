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
  logFile.open(logFileName, std::fstream::out);
  if (logFile.fail())
    std::cerr << logFileName << ": " << strerror(errno) << std::endl;
}

void Logger::closeLogFile(void) {
  if (logFile.is_open())
    logFile.close();
}

void Logger::setLogLevel(logLevel newLevel) { currentLevel = newLevel; }
void Logger::setLogOutput(logOutput newOutput) { currentOutput = newOutput; }

std::vector<std::string> Logger::getLogInfo(logLevel Level) {
  std::vector<std::string> logInfo;
  switch (Level) {
  case logLevel::Trace:
    logInfo.push_back("TRACE");
    logInfo.push_back(CYAN);
    return logInfo;
  case logLevel::Debug:
    logInfo.push_back("DEBUG");
    logInfo.push_back(GREEN);
    return logInfo;
  case logLevel::Info:
    logInfo.push_back("INFO");
    logInfo.push_back(BLUE);
    return logInfo;
  case logLevel::Warn:
    logInfo.push_back("WARNING");
    logInfo.push_back(YELLOW);
    return logInfo;
  case logLevel::Error:
    logInfo.push_back("ERROR");
    logInfo.push_back(RED);
    return logInfo;
  case logLevel::Critical:
    logInfo.push_back("CRITICAL");
    logInfo.push_back(RED);
    return logInfo;
  default:
    logInfo.push_back("UNKNOWN");
    logInfo.push_back(WHITE);
    return logInfo;
  }
}

std::string Logger::getTimeStamp(void) const {
  time_t now = time(0);
  tm *timeInfo = localtime(&now);
  char timeStamp[20];
  strftime(timeStamp, sizeof(timeStamp),
    "%Y-%m-%d %H:%M:%S", timeInfo);
  return timeStamp;
}
