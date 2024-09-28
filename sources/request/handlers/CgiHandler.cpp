#include <CgiHandler.hpp>
#include <Client.hpp>

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
  //closePipeFds();
}

void CgiHandler::closePipeFds(void) {
  LOG_TRACE("Closing file descriptors");
  if (inPipeFd[Fd::Read] != -1) {
    close(inPipeFd[Fd::Read]);
  }
  if (inPipeFd[Fd::Write] != -1) {
    close(inPipeFd[Fd::Write]);
  }
  if (outPipeFd[Fd::Read] != -1) {
    close(outPipeFd[Fd::Read]);
  }
  if (outPipeFd[Fd::Write] != -1) {
    close(outPipeFd[Fd::Write]);
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

void CgiHandler::executeCgiScript(void) {
  LOG_TRACE("Log entry from child process");
  std::vector<char*> argv = convertStringToChar(args);
  std::vector<char*> envp = convertStringToChar(envps);
  if (dup2(outPipeFd[Fd::Write], STDOUT_FILENO) == -1) {
    cgiError("Could not duplicate pipe fd");
  }
  if (dup2(inPipeFd[Fd::Read], STDIN_FILENO) == -1) {
    cgiError("Could not duplicate pipe fd");
  }
  close(outPipeFd[Fd::Write]);
  close(inPipeFd[Fd::Read]);
  if (execve(argv[0], argv.data(), envp.data()) == -1) {
    cgiError("Failed to execute CGI script");
  }
}

bool CgiHandler::isParentProcess(void) const {
  return this->pid != 0 ? true : false;
}

bool CgiHandler::isChildProcess(void) const {
  return this->pid == 0 ? true : false;
}

void CgiHandler::forkChildProcess(void) {
  LOG_TRACE("Forking new child process");
  this->pid = fork();
  if (this->pid == -1) {
    cgiError("Could not create child process");
  } else if (isChildProcess()) {
    LOG_DEBUG("Child pid:", getpid());
    close(outPipeFd[Fd::Read]);
    close(inPipeFd[Fd::Write]);
    executeCgiScript();
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
  cgiParam.start = std::chrono::steady_clock::now();
  g_CgiParams.push_back(cgiParam);
}

//edited
bool CgiHandler::isValidScript(void) const {
  struct stat s;
  if (!stat(cgi.c_str(), &s) && S_ISREG(s.st_mode) && !access(cgi.c_str(), R_OK)) {
    return true;
  }
  return false;
}

void CgiHandler::scriptLoader(void) {
  if (!isValidScript()) {
    cgiError("Could not open script");
  } else if (pipe(inPipeFd) == -1) {
    cgiError("Could not create pipe");
  } else if (pipe(outPipeFd) == -1) {
    cgiError("Could not create pipe");
  } else {
    forkChildProcess();
  }
}

void CgiHandler::runScript(void) {
  LOG_TRACE("Running new CGI instance");
  RuntimeException::tryCatch(&CgiHandler::scriptLoader, this);
}

void CgiHandler::executeRequest(Client& client) {
  LOG_TRACE("CgiHandler: executingRequest");
  cgi = client.getRes().getReqURI();

  std::string ext = cgi.substr(cgi.find_last_of(".") + 1);
  if (ext == "py")
    args.push_back(client.getRes().getServerConfig().cgiInterpreters["py"]);
  else if (ext == "php")
    args.push_back(client.getRes().getServerConfig().cgiInterpreters["php"]);
  else
    LOG_TRACE("Using binary:", cgi);
  clientFd = client.getFd();
  args.push_back(cgi);
  generateEnvpVector(client);
  runScript();
}
