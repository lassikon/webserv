#include <CgiHandler.hpp>
#include <Client.hpp>

// static std::map<pid_t, int> pids;

void CgiHandler::debugPrintCgiFd(void) {
  char buffer[256];
  LOG_DEBUG("Printing CGI fd contents");
  read(cgiFd, &buffer, sizeof(buffer));
  write(STDOUT_FILENO, &buffer, sizeof(buffer));
}

CgiHandler::CgiHandler(const Client& client) {
  cgi = "/run/media/jankku/Verbergen/dev/42/webserv/cgi-bin/hello.cgi";
  LOG_TRACE(Utility::getConstructor(*this));
  args.push_back(cgi);
  generateEnvpVector();
  (void)client;
  cgiFd = pipefd[Fd::Read];
}

CgiHandler::CgiHandler(void) {  // delete this
  LOG_TRACE(Utility::getConstructor(*this));
}

void CgiHandler::generateEnvpVector(void) {
  envps.push_back(std::string("REDIRECT_STATUS=") + "200");
  envps.push_back(std::string("REQUEST_METHOD=") + "FILLME");
  envps.push_back(std::string("SCRIPT_FILENAME=") + "FILLME");
  envps.push_back(std::string("PATH_INFO=") + "FILLME");
  envps.push_back(std::string("CONTENT_TYPE=") + "FILLME");
  envps.push_back(std::string("CONTENT_LENGTH=") + "FILLME");
  envps.push_back(std::string("QUERY_STRING=") + "FILLME");
  envps.push_back(std::string("HTTP_COOKIE=") + "FILLME");
}

CgiHandler::~CgiHandler(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  closePipeFds();
}

void CgiHandler::closePipeFds(void) {
  LOG_TRACE("Closing file descriptors");
  if (pipefd[Fd::Read] != -1) {
    close(pipefd[Fd::Read]);
  }
  if (pipefd[Fd::Write] != -1) {
    close(pipefd[Fd::Write]);
  }
  if (cgiFd != -1) {
    close(cgiFd);
  }
}

void CgiHandler::killAllChildPids(void) {
  for (auto& cgiParam : g_CgiParams) {
    kill(cgiParam.pid, SIGKILL);
  }
}

void CgiHandler::waitChildProcess(void) {
  //waitpid(this->pid, &wstat, WNOHANG);
  waitpid(this->pid, &wstat, 0);
  if (WIFSIGNALED(wstat) != 0) {
    g_ExitStatus = (int)Error::Signal + WTERMSIG(wstat);
  } else if (WIFEXITED(wstat)) {
    g_ExitStatus = WEXITSTATUS(wstat);
    close(pipefd[Fd::Write]);
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
  if (dup2(pipefd[Fd::Write], STDOUT_FILENO) == -1) {
    cgiError("Could not duplicate pipe fd");
  }
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

// const pid_t& CgiHandler::addNewProcessId(void) noexcept {
//   g_Pids.insert(std::make_pair(fork(), pipefd[Fd::Read]);
//   return g_Pids[pids.size() - 1];
// }

void CgiHandler::forkChildProcess(void) {
  LOG_TRACE("Forking new child process");
  // this->pid = addNewProcessId();
  this->pid = fork();
  if (this->pid == -1) {
    cgiError("Could not create child process");
  } else if (isChildProcess()) {
    LOG_DEBUG("Child pid:", getpid());
    //close(pipefd[Fd::Read]);
    executeCgiScript();
  } else if (isParentProcess()) {
    LOG_DEBUG("Parent pid:", getpid());
   // cgiFd = pipefd[Fd::Read];
    CgiParams cgiParam;
    cgiParam.pid = this->pid;
    cgiParam.fd = pipefd[Fd::Read];
    cgiParam.write = pipefd[Fd::Write];
    cgiParam.clientFd = clientFd;
    cgiParam.isExited = false;
    cgiParam.start = std::chrono::steady_clock::now();
    g_CgiParams.push_back(cgiParam);
  }
}

bool CgiHandler::isValidScript(void) const {
  struct stat s;
  if (!stat(cgi.c_str(), &s) && S_ISREG(s.st_mode) && !access(cgi.c_str(), X_OK)) {
    return true;
  }
  return false;
}

void CgiHandler::scriptLoader(void) {
  //cgiError("whatever error");
  if (!isValidScript()) {
    cgiError("Could not open script");
  } else if (pipe(pipefd) == -1) {
    cgiError("Could not create pipe");
  } else {
    forkChildProcess();
   // waitChildProcess();
  }
}

void CgiHandler::runScript(void) {
  LOG_TRACE("Running new CGI instance");
  Exception::tryCatch(&CgiHandler::scriptLoader, this);
  //debugPrintCgiFd();
}

void CgiHandler::executeRequest(Client& client) {
  LOG_TRACE("CgiHandler: executingRequest");
  cgi = client.getRes().getReqURI();
  LOG_TRACE("Using binary:", cgi);
  clientFd = client.getFd();
  args.push_back(cgi);
  generateEnvpVector();
  runScript();
}