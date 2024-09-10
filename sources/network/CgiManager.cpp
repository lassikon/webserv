#include <CgiManager.hpp>

std::vector<pid_t> CgiManager::pids;

void CgiManager::debugPrintCgiFd(void) {
  char buffer[256];
  LOG_DEBUG("Printing CGI fd contents");
  read(cgiFd, &buffer, sizeof(buffer));
  write(STDOUT_FILENO, &buffer, sizeof(buffer));
}

CgiManager::CgiManager(const Client& client) {
  cgi = "/run/media/jankku/Verbergen/dev/42/webserv/cgi-bin/hello.cgi";
  LOG_TRACE(Utility::getConstructor(*this));
  args.push_back(cgi);
  generateEnvpVector();
  (void)client;
}

CgiManager::CgiManager(void) : pipefd{-1, -1}, cgiFd(-1) {  // delete this
  cgi = "/run/media/jankku/Verbergen/dev/42/webserv/cgi-bin/hello.cgi";
  LOG_TRACE(Utility::getDeconstructor(*this));
  LOG_TRACE("Using binary:", cgi);
  args.push_back(cgi);
  generateEnvpVector();
}

void CgiManager::generateEnvpVector(void) {
  envps.push_back(std::string("REDIRECT_STATUS=") + "200");
  envps.push_back(std::string("REQUEST_METHOD=") + "FILLME");
  envps.push_back(std::string("SCRIPT_FILENAME=") + "FILLME");
  envps.push_back(std::string("PATH_INFO=") + "FILLME");
  envps.push_back(std::string("CONTENT_TYPE=") + "FILLME");
  envps.push_back(std::string("CONTENT_LENGTH=") + "FILLME");
  envps.push_back(std::string("QUERY_STRING=") + "FILLME");
  envps.push_back(std::string("HTTP_COOKIE=") + "FILLME");
}

CgiManager::~CgiManager(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  closePipeFds();
}

void CgiManager::closePipeFds(void) {
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

void CgiManager::killAllChildPids(void) {
  for (const auto& pid : pids) {
    if (pid != -1) {
      LOG_DEBUG("Terminating child process:", pid);
      kill(pid, SIGKILL);
    }
  }
}

void CgiManager::waitChildProcess(void) {
  waitpid(this->pid, &wstat, WNOHANG);
  if (WIFSIGNALED(wstat) != 0) {
    g_ExitStatus = (int)Error::Signal + WTERMSIG(wstat);
  } else if (WIFEXITED(wstat)) {
    g_ExitStatus = WEXITSTATUS(wstat);
  }
}

std::vector<char*> CgiManager::convertStringToChar(std::vector<std::string>& vec) {
  std::vector<char*> vector{};
  for (auto& string : vec) {
    vector.push_back(&string.front());
  }
  vector.push_back(nullptr);
  return vector;
}

void CgiManager::executeCgiScript(void) {
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

bool CgiManager::isParentProcess(void) const {
  return this->pid != 0 ? true : false;
}

bool CgiManager::isChildProcess(void) const {
  return this->pid == 0 ? true : false;
}

const pid_t& CgiManager::addNewProcessId(void) noexcept {
  pids.push_back(fork());
  return pids[pids.size() - 1];
}

void CgiManager::forkChildProcess(void) {
  LOG_TRACE("Forking new child process");
  this->pid = addNewProcessId();
  if (this->pid == -1) {
    cgiError("Could not create child process");
  } else if (isChildProcess()) {
    LOG_DEBUG("Child pid:", getpid());
    executeCgiScript();
  } else if (isParentProcess()) {
    LOG_DEBUG("Parent pid:", getpid());
    cgiFd = pipefd[Fd::Read];
  }
}

bool CgiManager::isValidScript(void) const {
  struct stat s;
  if (!stat(cgi.c_str(), &s) && S_ISREG(s.st_mode) && !access(cgi.c_str(), X_OK)) {
    return true;
  }
  return false;
}

void CgiManager::scriptLoader(void) {
  cgiError("whatever error");
  if (!isValidScript()) {
    cgiError("Could not open script");
  } else if (pipe(pipefd) == -1) {
    cgiError("Could not create pipe");
  } else {
    forkChildProcess();
    waitChildProcess();
  }
}

void CgiManager::runScript(void) {
  LOG_TRACE("Running new CGI instance");
  Exception::tryCatch(&CgiManager::scriptLoader, this);
  /* debugPrintCgiFd(); */
}
