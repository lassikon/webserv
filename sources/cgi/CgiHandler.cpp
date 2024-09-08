#include <CgiHandler.hpp>

std::vector<pid_t> CgiHandler::pids;

/* : cgiFd(-1), pipefd{-1, -1} */

CgiHandler::CgiHandler(const Client& client) {
  cgi = "/run/media/jankku/Verbergen/dev/42/webserv/cgi-bin/hello.cgi";
  LOG_TRACE(Utility::getConstructor(*this));
  args.push_back(cgi);
  generateEnvpMap();
  (void)client;
}

CgiHandler::CgiHandler(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  cgi = "/run/media/jankku/Verbergen/dev/42/webserv/cgi-bin/hello.cgi";
  LOG_TRACE("Using binary:", cgi);
  args.push_back(cgi);
  generateEnvpMap();
}

void CgiHandler::generateEnvpMap(void) {
  envpmap = {{"REDIRECT_STATUS=", "200"},    {"REQUEST_METHOD=", "FILLME"},
             {"SCRIPT_FILENAME=", "FILLME"}, {"PATH_INFO=", "FILLME"},
             {"CONTENT_TYPE=", "FILLME"},    {"CONTENT_LENGTH=", "FILLME"},
             {"QUERY_STRING=", "FILLME"},    {"HTTP_COOKIE=", "FILLME"}};
}

CgiHandler::~CgiHandler(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  closePipeFds();
}

void CgiHandler::closePipeFds(void) {
  LOG_DEBUG("Closing file descriptors");
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
  for (const auto& pid : pids) {
    if (pid != -1) {
      LOG_DEBUG("Terminating child process:", pid);
      kill(pid, SIGTERM);
    }
  }
}

void CgiHandler::waitChildProcess(void) {
  waitpid(this->pid, &wstat, WNOHANG);
  if (WIFSIGNALED(wstat) != 0) {
    g_ExitStatus = (int)Error::Signal + WTERMSIG(wstat);
  } else if (WIFEXITED(wstat)) {
    g_ExitStatus = WEXITSTATUS(wstat);
  }
}

void CgiHandler::convertEnvpMapToVector(void) {
  for (const auto& [envp, value] : envpmap) {
    this->envps.push_back(envp + value);
  }
}

std::vector<char*> CgiHandler::createEnvpArray(void) {
  convertEnvpMapToVector();
  std::vector<char*> vector{};
  for (auto& string : this->envps) {
    vector.push_back(&string.front());
  }
  return vector;
}

std::vector<char*> CgiHandler::createArgvArray(void) {
  std::vector<char*> vector{};
  for (auto& string : this->args) {
    vector.push_back(&string.front());
  }
  return vector;
}

void CgiHandler::executeCgiScript(void) {
  LOG_TRACE("Log entry from child process");
  std::vector<char*> argv = createArgvArray();
  std::vector<char*> envp = createEnvpArray();
  LOG_TRACE(argv[0], argv[1], envp[1]);
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

const pid_t& CgiHandler::addNewProcessId(void) noexcept {
  pids.push_back(fork());
  return pids[pids.size() - 1];
}

void CgiHandler::forkChildProcess(void) {
  LOG_TRACE("Forking new child process");
  this->pid = addNewProcessId();
  if (this->pid == -1) {
    cgiError("Could not create child process");
  } else if (isChildProcess()) {
    LOG_DEBUG("Child pid:", getpid());
    executeCgiScript();
  } else if (isParentProcess()) {
    LOG_DEBUG("Parent pid:", getpid());
    if (dup2(pipefd[Fd::Read], cgiFd) == -1) {
      cgiError("Could not duplicate pipe fd");
    }
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
  if (!isValidScript()) {
    cgiError("Could not open script");
  } else if (pipe(pipefd) == -1) {
    cgiError("Could not create pipe");
  } else {
    forkChildProcess();
    waitChildProcess();
  }
}

void CgiHandler::runScript(void) {
  LOG_TRACE("Running new CGI instance");
  Exception::tryCatch(&CgiHandler::scriptLoader, this);
}

void CgiHandler::childTimeout(int sigNum) noexcept {
  LOG_DEBUG("Child process terminated by timeout");
  kill(this->pid, SIGTERM);
  (void)sigNum;
}

void CgiHandler::childSuccess(int sigNum) noexcept {
  LOG_DEBUG("Child process finished execution");
  (void)sigNum;
}
