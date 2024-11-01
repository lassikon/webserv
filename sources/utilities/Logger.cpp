#include <Logger.hpp>

// map to hold all project class names for filtering
std::unordered_map<std::string, bool> Logger::classFilter = {
  // config
  {"Config", false},
  {"ConfigInitializer", false},

  // config/setters
  {"RouteDirectiveSetter", false},
  {"ServeDefaultFileAction", false},

  // network
  {"Client", false},
  {"Server", false},
  {"Socket", false},
  {"PollManager", false},
  {"ServersManager", false},
  {"SessionManager", false},

  // request
  {"Request", false},
  {"ProcessTree", false},
  {"ProcessTreeBuilder", false},

  // request/handlers
  {"GetHandler", false},
  {"PostHandler", false},
  {"DeleteHandler", false},
  {"CgiHandler", false},

  // response
  {"Response", false},

  // response/actions
  {"ServeDefaultFileAction", false},
  {"ServeDirectoryListingAction", false},
  {"ServeFileAction", false},
  {"ServeIndexAction", false},
  {"ServeRedirectAction", false},

  // utilities
  {"Logger", false},
  {"Signal", false},
  {"Utility", false},
};

// other static header variables declared for compiler
const char* Logger::fileName = "webserv.log";
std::ofstream Logger::logFile;
logOutput Logger::currentOutput;
logLevel Logger::currentLevel;
std::array<bool, 4> Logger::enabledDetail;

// define default settings for the logger
void Logger::loadDefaults(void) {
  currentLevel = logLevel::Trace;
  currentOutput = logOutput::Both;
  setLogDetails(true, true, true, true);
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

// set functions to change logging details (multiple or single)
void Logger::setLogDetails(bool time, bool file, bool func, bool line) {
  setLogDetail(logDetail::Time, time);
  setLogDetail(logDetail::File, file);
  setLogDetail(logDetail::Func, func);
  setLogDetail(logDetail::Line, line);
}

void Logger::setLogDetail(logDetail index, bool value) {
  enabledDetail.at((int)index) = value;
}

// function to generate and format logging time stamp
std::string Logger::getDateTimeStamp(void) {
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
  if (currentOutput != logOutput::FileOnly) {
    console << logEntry.str() << RESET << std::endl;
  }
  if (currentOutput != logOutput::ConsoleOnly && logFile.is_open()) {
    logFile << logEntry.str() << RESET << std::endl;
  }
}

// helper function to filter base file name from /path/file.cpp
std::string Logger::filterClassName(std::string& fileName) {
  fileName = fileName.substr(0, fileName.find_last_of('.'));
  return fileName.substr(fileName.find_last_of('/') + 1);
}

// checker function to see if filtered class name is in map
bool Logger::isFiltered(std::string& fileName) noexcept {
  const std::string className = filterClassName(fileName);
  auto it = classFilter.find(className);
  if (it != classFilter.end()) {
    return it->second;
  } else {
    return false;
  }
}
