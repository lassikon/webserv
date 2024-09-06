#include <Exception.hpp>
#include <Logger.hpp>

Logger::Logger(void) {
  loadDefaults();
}

Logger::~Logger(void) {
  closeLogFile();
}

void Logger::loadDefaults(void) {
  currentLevel = logLevel::Debug;
  currentOutput = logOutput::Both;
  setLogDetails(false, false, true, true);
  if (currentOutput != logOutput::ConsoleOnly)
    createLogFile();
}

void Logger::createLogFile(void) {
  logFile.open(fileName, std::ios_base::app);
  if (logFile.fail())
    LOG_WARN("Could not open file:", fileName, STRERROR);
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

std::string Logger::getDateTimeStamp(void) const {
  auto now = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  oss << std::put_time(std::localtime(&tt), "%Y-%m-%d %X");
  return oss.str();
}

void Logger::insertLogDetails(std::ostringstream& log, std::string src, const char* fn, int line) {
  if (enabledDetail[(int)logDetail::Time]) {
    log << "[" << getDateTimeStamp() << "]";
  }
  if (enabledDetail[(int)logDetail::File]) {
    log << "[" << src.substr(src.find_last_of('/') + 1, 4);
  }
  if (enabledDetail[(int)logDetail::Func]) {
    log << ":" << fn;
  }
  if (enabledDetail[(int)logDetail::Line]) {
    log << ":" << line << "]";
  }
}

void Logger::printLogEntry(std::ostream& console, std::ostringstream& logEntry) {
  if (currentOutput != logOutput::FileOnly) {
    console << logEntry.str() << RESET << std::endl;
  }
  if (currentOutput != logOutput::ConsoleOnly && logFile.is_open()) {
    logFile << logEntry.str() << RESET << std::endl;
  }
}
