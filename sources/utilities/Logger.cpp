#include <Logger.hpp>

// static variables declared for compiler
const char* Logger::fileName = "webserv.log";
std::ofstream Logger::logFile;
logOutput Logger::currentOutput;
logLevel Logger::currentLevel;
std::array<bool, 4> Logger::enabledDetail;

// define default settings for the logger
void Logger::loadDefaults(void) {
  currentLevel = logLevel::Info;
  currentOutput = logOutput::ConsoleOnly;
  setLogDetails(true, false, false, false);
  if (currentOutput != logOutput::ConsoleOnly) {
    createLogFile();
  }
}

void Logger::createLogFile(void) noexcept {
  logFile.open(fileName, std::ios_base::app);
  if (logFile.fail()) {
    LOG_WARN("Could not open file:", fileName, strerror(errno));
  }
}

void Logger::closeLogFile(void) noexcept {
  if (logFile.is_open())
    logFile.close();
}

void Logger::setLogOutput(const logOutput& newOutput) {
  currentOutput = newOutput;
}

void Logger::setLogLevel(const logLevel& newLevel) {
  currentLevel = newLevel;
}

void Logger::setLogDetail(logDetail index, bool value) {
  enabledDetail.at((int)index) = value;
}

// set functions to change logging details (multiple or single)
void Logger::setLogDetails(bool time, bool file, bool func, bool line) {
  setLogDetail(logDetail::Time, time);
  setLogDetail(logDetail::File, file);
  setLogDetail(logDetail::Func, func);
  setLogDetail(logDetail::Line, line);
}

// function to generate and format logging time stamp
std::string Logger::getDateTimeStamp(void) {
  auto now = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(now);
  std::ostringstream oss;
  oss << std::put_time(std::localtime(&tt), "%X");
  return oss.str();
}

void Logger::insertLogDetails(std::ostringstream& log, std::string src, const char* fn, int line) {
  if (enabledDetail[(int)logDetail::Time]) {
    log << "[" << getDateTimeStamp() << "]";
  }
  if (enabledDetail[(int)logDetail::File]) {
    log << filterClassName(src);
  }
  if (enabledDetail[(int)logDetail::Func]) {
    log << ":" << fn;
  }
  if (enabledDetail[(int)logDetail::Line]) {
    log << ":" << line << "]";
  }
}

// helper fuction to print log entry into target ostream and ostringstream
void Logger::printLogEntry(std::ostream& console, std::ostringstream& logEntry) {
  std::ostringstream logFinal;
  logFinal << logEntry.str() << RESET << '\n';
  if (currentOutput != logOutput::FileOnly) {
    console << logFinal.str();
  }
  if (currentOutput != logOutput::ConsoleOnly && logFile.is_open()) {
    logFile << logFinal.str();
  }
}

// helper function to filter base file name from /path/file.cpp
std::string Logger::filterClassName(std::string& fileName) {
  fileName = fileName.substr(0, fileName.find_last_of('.'));
  return fileName.substr(fileName.find_last_of('/') + 1);
}
