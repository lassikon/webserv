#include <Utility.hpp>

#include <NetworkException.hpp>

static std::map<std::string, std::string> mimeTypes = {
  {"html", "text/html"}, {"css", "text/css"},   {"js", "application/javascript"},
  {"png", "image/png"},  {"jpg", "image/jpeg"}, {"jpeg", "image/jpeg"},
  {"gif", "image/gif"},  {"txt", "text/plain"},
};

std::string Utility::trimWhitespaces(std::string& line) {
  size_t pos = line.find_first_not_of(" \t");
  if (pos != std::string::npos)
    line = line.substr(pos);
  pos = line.find_last_not_of(" \t");
  if (pos != std::string::npos)
    line = line.substr(0, pos + 1);
  return line;
}

std::string Utility::trimComments(std::string& line) {
  size_t pos = line.find("#");
  if (pos != std::string::npos)
    line = line.substr(0, pos);
  return line;
}

std::vector<char> Utility::readFile(std::string& path) {
  std::ifstream file(path, std::ios::binary | std::ios::in);
  if (!file.is_open()) {
    LOG_ERROR("Failed to open file:", path);
    return std::vector<char>();
  }
  std::vector<char> content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
  file.close();
  return content;
}

std::filesystem::path Utility::getExePath(std::filesystem::path& path) {
  std::filesystem::path currentPath = std::filesystem::current_path();
  std::filesystem::path exePath = currentPath / "webserv";
  if (!std::filesystem::exists(exePath)) {
    LOG_ERROR("Parse: Could not find executable at ", exePath);
    return (path);
  }
  path = std::filesystem::canonical(exePath).parent_path();
  return (path);
}

std::string Utility::getMimeType(std::string& extension) {
  auto it = mimeTypes.find(extension);
  if (it != mimeTypes.end()) {
    return it->second;
  }
  return "application/octet-stream";
}

void Utility::setNonBlocking(int& fd) {
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1) {
    LOG_ERROR("Failed to get file descriptor flags");
    return;
  }
  if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
    LOG_ERROR("Failed to set file descriptor to non-blocking");
  }
}

void Utility::setCloseOnExec(int& fd) {
  int flags = fcntl(fd, F_GETFD, 0);
  if (flags == -1) {
    LOG_ERROR("Failed to get file descriptor flags");
    return;
  }
  if (fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1) {
    LOG_ERROR("Failed to set file descriptor to close-on-exec");
  }
}

bool Utility::isCgiFd(int fd) {
  for (auto& cgi : g_CgiParams) {
    if (cgi.outReadFd == fd || cgi.inWriteFd == fd) {
      return true;
    }
  }
  return false;
}

int Utility::getClientFdFromCgiParams(int fd) {
  for (auto& cgi : g_CgiParams) {
    if (cgi.outReadFd == fd || cgi.inWriteFd == fd) {
      return cgi.clientFd;
    }
  }
  return -1;
}

int Utility::getOutReadFdFromClientFd(int fd) {
  for (auto& cgi : g_CgiParams) {
    if (cgi.clientFd == fd) {
      return cgi.outReadFd;
    }
  }
  return -1;
}

int Utility::getInWriteFdFromClientFd(int fd) {
  for (auto& cgi : g_CgiParams) {
    if (cgi.clientFd == fd) {
      return cgi.inWriteFd;
    }
  }
  return -1;
}

bool Utility::isOutReadFd(int fd) {
  for (auto& cgi : g_CgiParams) {
    if (cgi.outReadFd == fd) {
      return true;
    }
  }
  return false;
}

bool Utility::isInWriteFd(int fd) {
  for (auto& cgi : g_CgiParams) {
    if (cgi.inWriteFd == fd) {
      return true;
    }
  }
  return false;
}

bool Utility::getIsTimeout(int fd) {
  for (auto& cgi : g_CgiParams) {
    if (cgi.outReadFd == fd || cgi.inWriteFd == fd) {
      return cgi.isTimeout;
    }
  }
  return false;
}

bool Utility::getIsFailed(int fd) {
  for (auto& cgi : g_CgiParams) {
    if (cgi.outReadFd == fd || cgi.inWriteFd == fd) {
      return cgi.isFailed;
    }
  }
  return false;
}

bool Utility::signalReceived(void) noexcept {
  return g_ExitStatus == 0 ? false : true;
}

size_t Utility::convertSizetoBytes(std::string& size) {
  size_t bytes = 0;
  size_t multiplier = 1;
  if (size.back() == 'k' || size.back() == 'K') {
    multiplier = 1024;
    size.pop_back();
  } else if (size.back() == 'm' || size.back() == 'M') {
    multiplier = 1024 * 1024;
    size.pop_back();
  } else if (size.back() == 'g' || size.back() == 'G') {
    multiplier = 1024 * 1024 * 1024;
    size.pop_back();
  }
  try {
    bytes = std::stoull(size) * multiplier;
  } catch (const std::invalid_argument&) {
    // Handle invalid argument
    bytes = 0;
  } catch (const std::out_of_range&) {
    // Handle out of range
    bytes = SIZE_MAX;
  }
  bytes = std::stoull(size) * multiplier;
  return bytes;
}

bool Utility::getLineVectoStr(std::vector<char>& buffer, std::string& line, size_t& curr,
                              size_t& end) {
  std::string str;
  while (curr != end) {
    if (buffer[curr] == '\n') {
      line = str;
      str.clear();
      curr++;
      return true;
    } else {
      str.push_back(buffer[curr]);
    }
    curr++;
  }
  if (!str.empty()) {
    line = str;
    return true;
  }
  return false;
}
