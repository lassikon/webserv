#include <CgiHandler.hpp>
#include <Client.hpp>
#include <NetworkException.hpp>

CgiHandler::CgiHandler(void) {
  LOG_TRACE(Utility::getConstructor(*this));
}

void CgiHandler::generateEnvpVector(Client& client) {
  envps.push_back(std::string("REQUEST_METHOD=") + client.getReq().getMethod());
  envps.push_back(std::string("SCRIPT_FILENAME=") + client.getRes().getReqURI());
  envps.push_back(std::string("PATH_INFO=") + client.getRes().getReqURI());
  envps.push_back(std::string("CONTENT_TYPE=") + client.getReq().getHeaders()["Content-Type"]);
  envps.push_back(std::string("CONTENT_LENGTH=") + std::to_string(client.getReq().getBodySize()));
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

std::vector<char*> CgiHandler::convertStringToChar(std::vector<std::string>& vec) {
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
  if (outPipeFd[Fd::Write] != -1) {
    if (dup2(outPipeFd[Fd::Write], STDOUT_FILENO) == -1) {
      exitError(2, "Could not duplicate pipe fd1");
    }
  }
  if (inPipeFd[Fd::Read] != -1) {
    if (dup2(inPipeFd[Fd::Read], STDIN_FILENO) == -1) {
      exitError(2, "Could not duplicate pipe fd2");
    }
  }
  close(outPipeFd[Fd::Write]);
  close(inPipeFd[Fd::Read]);
  std::filesystem::path path(client.getRes().getReqURI());
  std::string pathStr = path.parent_path().c_str();
  std::string pathMsg = "Changing execution directory to: " + pathStr;
  LOG_CGI(pathMsg);
  // chdir(path.parent_path().c_str());
  LOG_FATAL("Executing CGI script:", argv[0]);
  if (execve(argv[0], argv.data(), envp.data()) == -1) {
    exitError(2, "Could not duplicate pipe fd3");
  }
}

bool CgiHandler::isParentProcess(void) const {
  return this->pid != 0 ? true : false;
}

bool CgiHandler::isChildProcess(void) const {
  return this->pid == 0 ? true : false;
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
    close(outPipeFd[Fd::Write]);
    close(inPipeFd[Fd::Read]);
    setGlobal();
  }
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

bool CgiHandler::isValidScript(void) const {
  struct stat s;
  if (!stat(cgi.c_str(), &s) && !S_ISREG(s.st_mode)) {
    LOG_ERROR("File is a folder:", cgi);
    return false;
  }
  if (isBin) {
    if (access(cgi.c_str(), X_OK)) {
      LOG_ERROR("No permission to execute binary file:", cgi);
      return false;
    }
  } else {
    if (access(cgi.c_str(), R_OK)) {
      LOG_ERROR("No permission to execute CGI script", cgi);
      return false;
    }
  }
  return true;
}

void CgiHandler::scriptLoader(Client& client) {
  if (!isValidScript()) {
    throw httpBadGateway(client, "Failed to execute CGI script");
  }
  if (client.getReq().getMethod() == "POST") {
    if (pipe(inPipeFd) == -1) {
      throw httpBadGateway(client, "Failed to create pipe");
    }
  }
  if (pipe(outPipeFd) == -1) {
    throw httpBadGateway(client, "Failed to create pipe");
  }
  LOG_DEBUG("Pipe out fds created:", outPipeFd[Fd::Read], outPipeFd[Fd::Write]);
  LOG_DEBUG("Pipe in fds created:", inPipeFd[Fd::Read], inPipeFd[Fd::Write]);
  forkChildProcess(client);
}

void CgiHandler::runScript(Client& client) {
  LOG_TRACE("Running new CGI instance");
  scriptLoader(client);
}

void CgiHandler::executeRequest(Client& client) {
  LOG_TRACE("CgiHandler: executingRequest");
  cgi = client.getRes().getReqURI();
  std::string ext = cgi.substr(cgi.find_last_of(".") + 1);
  if (ext == "py")
    args.push_back(client.getRes().getServerConfig().cgiInterpreters["py"]);
  else if (ext == "php")
    args.push_back(client.getRes().getServerConfig().cgiInterpreters["php"]);
  else {
    LOG_TRACE("Using binary:", cgi);
    isBin = true;
  }
  clientFd = client.getFd();
  args.push_back(cgi);
  generateEnvpVector(client);
  runScript(client);
}
