#include <ServerDirectiveSetter.hpp>

void ServerDirectiveSetter::handleDirective(void* data, std::string& key, std::string& value,
                                            int& lineNumber) {
  ServerConfig* server = static_cast<ServerConfig*>(data);
  if (key == "server_ip") {
    setIP(*server, value, lineNumber);
  } else if (key == "server_name") {
    setServerName(*server, value, lineNumber);
  } else if (key == "server_port") {
    setPort(*server, value, lineNumber);
  } else if (key == "error_page") {
    setErrorPages(*server, value, lineNumber);
  } else if (key == "client_body_size_limit") {
    setClientBodySizeLimit(*server, value, lineNumber);
  } else if (key == "cgi_interpreter") {
    setCgiInterpreters(*server, value, lineNumber);
  } else {
    LOG_WARN("Parse: Invalid directive,", key, " at line ", lineNumber);
  }
}

// server struct setters
void ServerDirectiveSetter::setIP(ServerConfig& server, std::string& value, int& lineNumber) {
  const std::regex ipPattern("^(?:[0-9]{1,3}\\.){3}[0-9]{1,3}$");
  if (!std::regex_match(value, ipPattern)) {
    LOG_WARN("Parse: Invalid IP,", value, " at line", lineNumber);
    return;
  }
  if (!server.ipAddress.empty())
    LOG_WARN("Parse: IP already set, updating with line", lineNumber);
  server.ipAddress = value;
}

void ServerDirectiveSetter::setServerName(ServerConfig& server, std::string& value,
                                          int& lineNumber) {
  const std::regex serverNamePattern("^(\\w)[\\w-]{0,61}(\\w)(\\.[\\w-]{1,63})*$");
  LOG_TRACE("Server name:", value);
  if (!std::regex_match(value, serverNamePattern)) {
    LOG_WARN("Parse: Invalid server name,", value, " at line ", lineNumber);
    return;
  }
  if (!server.serverName.empty())
    LOG_WARN("Parse: Server name already set, updating with line", lineNumber);
  server.serverName = value;
}

void ServerDirectiveSetter::setPort(ServerConfig& server, std::string& value, int& lineNumber) {
  const std::regex port_pattern("^[0-9]+$");
  if (!std::regex_match(value, port_pattern)) {
    LOG_WARN("Parse: Invalid port,", value, " at line", lineNumber);
    return;
  } else if (std::stoi(value) < 0 || std::stoi(value) > 65535) {
    LOG_WARN("Parse: Invalid port range,", value, " at line", lineNumber);
    return;
  }
  if (server.port != 0)
    LOG_WARN("Parse: Port already set, updating with line", lineNumber);
  server.port = std::stoi(value);
}

// relative to cwd
void ServerDirectiveSetter::setErrorPages(ServerConfig& server, std::string& value,
                                          int& lineNumber) {
  std::stringstream ss(value);
  std::string error;
  std::getline(ss, error, ' ');
  const std::regex error_pattern("^[0-9]+$");
  if (!std::regex_match(error, error_pattern)) {
    LOG_WARN("Parse: Invalid error code,", error, " at line", lineNumber);
    return;
  }
  int error_code = std::stoi(error);
  if (server.pagesDefault.find(error_code) == server.pagesDefault.end()) {
    LOG_WARN("Parse: Invalid Custom Error page at", lineNumber);
    return;
  }
  std::getline(ss, error, ' ');
  std::filesystem::path exePath;
  exePath = Utility::getExePath(exePath);
  std::filesystem::path errorPath = exePath.append(error);
  if (!std::filesystem::exists(errorPath))
    LOG_WARN("Parse: Error page path not found,", error, " at line", lineNumber);
  else
    server.pagesCustom[error_code] = error;
}

// client body size limit is set in bytes, kilobytes k,K, megabytes m,M, gigabytes g,G
void ServerDirectiveSetter::setClientBodySizeLimit(ServerConfig& server, std::string& value,
                                                   int& lineNumber) {
  const std::regex size_pattern("^[0-9]+[kKmMgG]?$");
  if (!std::regex_match(value, size_pattern)) {
    LOG_WARN("Parse: Invalid client body size limit,", value, " at line", lineNumber);
    return;
  }
  if (!server.clientBodySizeLimit.empty())
    LOG_WARN("Parse: Client body size limit already set, updating with line", lineNumber);
  server.clientBodySizeLimit = value;
}

void ServerDirectiveSetter::setCgiInterpreters(ServerConfig& server, std::string& value,
                                               int& lineNumber) {
  std::stringstream ss(value);
  std::string interpreter;
  std::string extension;
  std::getline(ss, extension, ' ');
  std::getline(ss, interpreter, ' ');
  if (interpreter.empty() || extension.empty()) {
    LOG_WARN("Parse: Invalid CGI interpreter,", value, " at line", lineNumber);
    return;
  }
  if (server.cgiInterpreters.find(extension) != server.cgiInterpreters.end())
    LOG_WARN("Parse: CGI interpreter already set for extension,", extension, " at line",
             lineNumber);
  if (!std::filesystem::exists(interpreter))
    LOG_WARN("Parse: CGI interpreter path not found,", interpreter, " at line", lineNumber);
  else
    server.cgiInterpreters[extension] = interpreter;
}