#include <Logger.hpp>
#include <Utility.hpp>

static std::map<std::string, std::string> mimeTypes = {
    {"html", "text/html"},
    {"css", "text/css"},
    {"js", "application/javascript"},
    {"png", "image/png"},
    {"jpg", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"gif", "image/gif"},
    {"txt", "text/plain"},
};

std::string Utility::trimWhitespaces(std::string &line) {
  size_t pos = line.find_first_not_of(" \t");
  if (pos != std::string::npos)
    line = line.substr(pos);
  pos = line.find_last_not_of(" \t");
  if (pos != std::string::npos)
    line = line.substr(0, pos + 1);
  return line;
}

std::string Utility::trimComments(std::string &line) {
  size_t pos = line.find("#");
  if (pos != std::string::npos)
    line = line.substr(0, pos);
  return line;
}

std::vector<char> Utility::readFile(std::string &path) {
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

std::filesystem::path Utility::getExePath(std::filesystem::path &path) {
  std::filesystem::path currentPath = std::filesystem::current_path();
  std::filesystem::path exePath = currentPath / "webserv";
  if (!std::filesystem::exists(exePath)) {
    LOG_ERROR("Parse: Could not find executable at ", exePath);
    return (path);
  }
  path = std::filesystem::canonical(exePath).parent_path();
  return (path);
}

std::string Utility::getMimeType(std::string &extension) {
  auto it = mimeTypes.find(extension);
  if (it != mimeTypes.end()) {
    return it->second;
  }
  return "application/octet-stream";
}

int Utility::statusOk(void) noexcept { return !g_ExitStatus; }
