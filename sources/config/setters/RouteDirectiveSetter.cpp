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
  std::filesystem::path rootPath = std::filesystem::canonical(value);
  std::string root = rootPath.string();
  if (!std::filesystem::exists(root)) {
    LOG_WARN("Parse: Root path not found,", root, " at line", lineNumber);
    return;
  }

  if (root.compare(0, exePath.string().size(), exePath.string()) == 0) {
    if (!route.root.empty())
      LOG_WARN("Parse: Root already set, updating with line", lineNumber);
    route.root = root;
  } else {
    LOG_WARN("Parse: Root path is not allowed,", root, " at line", lineNumber);
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
    //sanitize the default file removing relative path
    if (defaultFile.find("..") != std::string::npos) {
      LOG_WARN("Parse: Invalid default file,", defaultFile, " at line", lineNumber);
      continue;
    }
    route.defaultFile.push_back(defaultFile);
  }
}

void RouteDirectiveSetter::setUploadPath(RouteConfig& route, std::string& value, int& lineNumber) {
  std::string webroot = route.root;
  if (webroot.empty()) {
    LOG_WARN("Parse: Root path not set, upload path not allowed at line", lineNumber);
    return;
  }
  std::filesystem::path uploadPath = webroot + value;
  if (!std::filesystem::exists(uploadPath)) {
    LOG_DEBUG("Parse: Upload path not found,", uploadPath.string(), " at line", lineNumber);
    LOG_DEBUG("Parse: Attempting to create upload path,", uploadPath.string());
    //create
    if (!std::filesystem::create_directories(uploadPath)) {
      LOG_WARN("Parse: Failed to create upload path,", uploadPath.string(), " at line", lineNumber);
    return;
    }
  }
  std::string upload = uploadPath.string();
  LOG_DEBUG("Upload path:", upload);
  if (!std::filesystem::exists(upload)) {
    LOG_DEBUG("Parse: Upload path not found,", upload, " at line", lineNumber);
    return;
  }
  if (!route.uploadPath.empty())
    LOG_WARN("Parse: Upload path already set, updating with line", lineNumber);
  route.uploadPath = upload;
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