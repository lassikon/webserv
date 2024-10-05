#include <RouteDirectiveSetter.hpp>

void RouteDirectiveSetter::handleDirective(void* data, std::string& key, std::string& value,
                                           int& lineNumber) {
  RouteConfig* route = static_cast<RouteConfig*>(data);
  if (key == "location") {
    setLocation(*route, value, lineNumber);
  } else if (key == "methods") {
    setMethods(*route, value, lineNumber);
  } else if (key == "root") {
    setRoot(*route, value, lineNumber);
  } else if (key == "directory_listing") {
    setDirectoryListing(*route, value, lineNumber);
  } else if (key == "default_file") {
    setDefaultFile(*route, value, lineNumber);
  } else if (key == "upload_path") {
    setUploadPath(*route, value, lineNumber);
  } else if (key == "redirect") {
    setRedirect(*route, value, lineNumber);
  } else if (key == "cgi") {
    setCgi(*route, value, lineNumber);
  } else {
    LOG_WARN("Parse: Invalid directive,", key, " at line ", lineNumber);
  }
}

// route struct setters
void RouteDirectiveSetter::setLocation(RouteConfig& route, std::string& value, int& lineNumber) {
  if (!route.location.empty())
    LOG_WARN("Parse: Location already set, updating with line", lineNumber);
  route.location = value;
}

void RouteDirectiveSetter::setMethods(RouteConfig& route, std::string& value, int& lineNumber) {
  std::stringstream ss(value);
  std::string method;
  while (std::getline(ss, method, ',')) {
    method = Utility::trimWhitespaces(method);
    if (method.empty())
      LOG_WARN("Parse: Invalid method,", method, " at line", lineNumber);
    else if (method != "GET" && method != "POST" && method != "DELETE") {
      LOG_WARN("Parse: Invalid method,", method, " at line", lineNumber);
      continue;
    } else {
      route.methods.push_back(method);
    }
  }
}
// must be an absolute path
void RouteDirectiveSetter::setRoot(RouteConfig& route, std::string& value, int& lineNumber) {
  std::filesystem::path exePath;
  exePath = Utility::getExePath(exePath);
  if (!std::filesystem::exists(value)) {
    LOG_WARN("Parse: Root path not found,", value, " at line", lineNumber);
    return;
  }

  if (value.compare(0, exePath.string().size(), exePath.string()) == 0) {
    if (!route.root.empty())
      LOG_WARN("Parse: Root already set, updating with line", lineNumber);
    route.root = value;
  } else {
    LOG_WARN("Parse: Root path is not allowed,", value, " at line", lineNumber);
  }
}

void RouteDirectiveSetter::setDirectoryListing(RouteConfig& route, std::string& value,
                                               int& lineNumber) {
  if (value == "on")
    route.directoryListing = true;
  else if (value == "off")
    route.directoryListing = false;
  else
    LOG_WARN("Parse: Invalid directory listing,", value, " at line", lineNumber);
}

void RouteDirectiveSetter::setDefaultFile(RouteConfig& route, std::string& value, int& lineNumber) {
  if (!route.defaultFile.empty())
    LOG_WARN("Parse: Default file already set, updating with line", lineNumber);
  std::stringstream ss(value);
  std::string defaultFile;
  while (std::getline(ss, defaultFile, ' ')) {
    route.defaultFile.push_back(defaultFile);
  }
}

void RouteDirectiveSetter::setUploadPath(RouteConfig& route, std::string& value, int& lineNumber) {
  if (!std::filesystem::exists(value)) {
    LOG_WARN("Parse: Upload path not found,", value, " at line", lineNumber);
    return;
  }
  if (!route.uploadPath.empty())
    LOG_WARN("Parse: Upload path already set, updating with line", lineNumber);
  route.uploadPath = value;
}
// either a path or a redirect url
void RouteDirectiveSetter::setRedirect(RouteConfig& route, std::string& value, int& lineNumber) {
  if (!route.redirect.empty())
    LOG_WARN("Parse: Redirect already set, updating with line", lineNumber);
  route.redirect = value;
}

void RouteDirectiveSetter::setCgi(RouteConfig& route, std::string& value, int& lineNumber) {
  (void)lineNumber;
  std::stringstream ss(value);
  std::string cgi;
  while (std::getline(ss, cgi, ',')) {
    cgi = Utility::trimWhitespaces(cgi);
    route.cgi.push_back(cgi);
  }
}