#include <CgiHandler.hpp>
#include <Client.hpp>
#include <NetworkException.hpp>

CgiHandler::CgiHandler(void) { LOG_TRACE(Utility::getConstructor(*this)); }

void CgiHandler::generateEnvpVector(Client& client) {
  envps.push_back(std::string("REQUEST_METHOD=") + client.getReq().getMethod());
  envps.push_back(std::string("SCRIPT_FILENAME=") +
                  client.getRes().getReqURI());
  envps.push_back(std::string("PATH_INFO=") + client.getRes().getReqURI());
  envps.push_back(std::string("CONTENT_TYPE=") +
                  client.getReq().getHeaders()["Content-Type"]);
  envps.push_back(std::string("CONTENT_LENGTH=") +
                  std::to_string(client.getReq().getBodySize()));
  envps.push_back(std::string("QUERY_STRING=") + client.getReq().getQuery());
  envps.push_back(std::string("HTTP_COOKIE=") + "FILLME");
}

CgiHandler::~CgiHandler(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  closePipeFds();
}

void CgiHandler::closePipeFds(void) {
  LOG_TRACE("Closing file descriptors");
  if (inPipeFd[Fd::Read] != -1) {
    close(inPipeFd[Fd::Read]);
    inPipeFd[Fd::Read] = -1;
  }
  if (inPipeFd[Fd::Write] != -1) {
    close(inPipeFd[Fd::Write]);
    inPipeFd[Fd::Write] = -1;
  }
  if (outPipeFd[Fd::Read] != -1) {
    close(outPipeFd[Fd::Read]);
    outPipeFd[Fd::Read] = -1;
  }
  if (outPipeFd[Fd::Write] != -1) {
    close(outPipeFd[Fd::Write]);
    outPipeFd[Fd::Write] = -1;
  }
}

void CgiHandler::killAllChildPids(void) {
  for (auto& cgiParam : g_CgiParams) {
    kill(cgiParam.pid, SIGKILL);
  }
}

std::vector<char*> CgiHandler::convertStringToChar(
    std::vector<std::string>& vec) {
  std::vector<char*> vector{};
  for (auto& string : vec) {
    vector.push_back(&string.front());
  }
  vector.push_back(nullptr);
  return vector;
}

void CgiHandler::exitError(int status, const std::string& message) {
  LOG_ERROR("CgiHandler:", message);
  std::exit(status);
}

void CgiHandler::executeCgiScript(Client& client) {
  std::vector<char*> argv = convertStringToChar(args);
  std::vector<char*> envp = convertStringToChar(envps);
  if (outPipeFd[Fd::Write] != -1 && dup2(outPipeFd[Fd::Write], STDOUT_FILENO) == -1) {
    exitError((int)RuntimeError::Cgi, "Could not duplicate pipe fd1");
  }
  if (inPipeFd[Fd::Read] != -1 && dup2(inPipeFd[Fd::Read], STDIN_FILENO) == -1) {
    exitError((int)RuntimeError::Cgi, "Could not duplicate pipe fd2");
  }
  std::filesystem::path path(client.getRes().getReqURI());
  std::string pathStr = path.parent_path().c_str();
  std::string pathMsg = "Changing execution directory to: " + pathStr;
  LOG_CGI(pathMsg);
  LOG_INFO("Executing CGI script:", argv[0]);
  if (execve(argv[0], argv.data(), envp.data()) == -1) {
    exitError((int)RuntimeError::Cgi, "Could not execute CGI script");
  }
}

bool CgiHandler::isParentProcess(void) const {
  return this->pid != 0 ? true : false;
}

bool CgiHandler::isChildProcess(void) const {
  return this->pid == 0 ? true : false;
}

void CgiHandler::setGlobal(void) {
  CgiParams cgiParam;
  cgiParam.pid = this->pid;
  cgiParam.outReadFd = outPipeFd[Fd::Read];
  cgiParam.outWriteFd = outPipeFd[Fd::Write];
  cgiParam.inReadFd = inPipeFd[Fd::Read];
  cgiParam.inWriteFd = inPipeFd[Fd::Write];
  cgiParam.clientFd = clientFd;
  cgiParam.isExited = false;
  cgiParam.isTimeout = false;
  cgiParam.isFailed = false;
  cgiParam.start = std::chrono::steady_clock::now();
  cgiParam.childExitStatus = -1;
  g_CgiParams.push_back(cgiParam);
}

void CgiHandler::forkChildProcess(Client& client) {
  LOG_TRACE("Forking new child process");
  this->pid = fork();
  if (this->pid == -1) {
    throw httpBadGateway(client, "Could not duplicate pipe fd");
  } else if (isChildProcess()) {
    LOG_DEBUG("Child pid:", getpid());
    close(outPipeFd[Fd::Read]);
    close(inPipeFd[Fd::Write]);
    executeCgiScript(client);
  } else if (isParentProcess()) {
    LOG_DEBUG("Parent pid:", getpid());
    envps.clear();
    setGlobal();
  }
}

bool CgiHandler::isValidPerm(void) const {
  namespace fs = std::filesystem;
  fs::perms perms = fs::status(cgi).permissions();
  if (isBin) {
    if ((perms & (fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec)) == fs::perms::none) {
      LOG_ERROR("No execute permission for binary file: ", cgi);
      return false;
    }
  } else {
    if ((perms & (fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read)) == fs::perms::none) {
      LOG_ERROR("No execute permission for binary file: ", cgi);
      return false;
    }
  }
  return true;
}

bool CgiHandler::isValidFile(void) const {
  if (!std::filesystem::exists(cgi)) {
    LOG_ERROR("File does not exist: ", cgi);
    return false;
  } else if (!std::filesystem::is_regular_file(cgi)) {
    LOG_ERROR("File is a folder or not a regular file: ", cgi);
    return false;
  }
  return true;
}

void CgiHandler::scriptLoader(Client& client) {
  LOG_TRACE("Running new CGI instance");
  if (!isValidFile() || !isValidPerm()) {
    throw httpBadGateway(client, "Failed to execute CGI script");
  }
  if (client.getReq().getMethod() == "POST") {
    if (pipe(inPipeFd) == -1) {
      throw httpBadGateway(client, "Failed to create pipe");
    }
    Utility::setCloseOnExec(inPipeFd[Read]);
    Utility::setCloseOnExec(inPipeFd[Write]);
    Utility::setNonBlocking(inPipeFd[Read]);
    Utility::setNonBlocking(inPipeFd[Write]);
  }
  if (pipe(outPipeFd) == -1) {
    throw httpBadGateway(client, "Failed to create pipe");
  }
  Utility::setCloseOnExec(outPipeFd[Read]);
  Utility::setCloseOnExec(outPipeFd[Write]);
  Utility::setNonBlocking(outPipeFd[Read]);
  Utility::setNonBlocking(outPipeFd[Write]);
  LOG_DEBUG("Pipe out fds created:", outPipeFd[Fd::Read], outPipeFd[Fd::Write]);
  LOG_DEBUG("Pipe in fds created:", inPipeFd[Fd::Read], inPipeFd[Fd::Write]);
  if (fcntl(outPipeFd[Read], F_GETFD) == -1) {
    LOG_ERROR("Failed to get file descriptor flags", outPipeFd[Read]);
  }
  if (fcntl(outPipeFd[Write], F_GETFD) == -1) {
    LOG_ERROR("Failed to get file descriptor flags", outPipeFd[Write]);
  }
  forkChildProcess(client);
}

void CgiHandler::executeRequest(Client& client) {
  LOG_TRACE("Executing CGI request");
  cgi = client.getRes().getReqURI();
  std::string ext = cgi.substr(cgi.find_last_of(".") + 1);
  if (ext == "py") {
    if (client.getRes().getServerConfig().cgiInterpreters.find("py") ==
        client.getRes().getServerConfig().cgiInterpreters.end()) {
      throw httpBadGateway(client, "Python interpreter not found");
    }
    args.push_back(client.getRes().getServerConfig().cgiInterpreters["py"]);
  } else if (ext == "php") {
    if (client.getRes().getServerConfig().cgiInterpreters.find("php") ==
        client.getRes().getServerConfig().cgiInterpreters.end()) {
      throw httpBadGateway(client, "PHP interpreter not found");
    }
    args.push_back(client.getRes().getServerConfig().cgiInterpreters["php"]);
  } else {
    LOG_DEBUG("Using CGI file:", cgi);
    isBin = true;
  }
  args.push_back(cgi);
  clientFd = client.getFd();
  generateEnvpVector(client);
  scriptLoader(client);
}
