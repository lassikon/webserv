#include <CgiHandler.hpp>

std::vector<pid_t> CgiHandler::pids;

CgiHandler::CgiHandler(const Request& request) {
  cgi = "/run/media/jankku/Verbergen/dev/42/webserv/cgi-bin/hello.cgi";
  LOG_TRACE(Utility::getConstructor(*this));
  args.push_back(cgi);
  (void)request;
}

CgiHandler::~CgiHandler(void) {
  LOG_TRACE(Utility::getDeconstructor(*this));
  closePipeFds();
}

void CgiHandler::closePipeFds(void) {
  if (pipefd[Fd::Read] != -1) {
    close(pipefd[Fd::Read]);
  }
  if (pipefd[Fd::Write] != -1) {
    close(pipefd[Fd::Write]);
  }
}

void CgiHandler::killAllChildPids(void) {
  for (auto const& pid : pids) {
    if (pid != -1) {
      LOG_DEBUG("Terminating child process:", pid);
      kill(pid, SIGTERM);
    }
  }
}

void CgiHandler::waitChildProcess(void) {
  waitpid(this->pid, &wstat, 0);
  if (WIFSIGNALED(wstat) != 0) {
    g_ExitStatus = (int)Error::Signal + WTERMSIG(wstat);
  } else {
    g_ExitStatus = WEXITSTATUS(wstat);
  }
}

std::vector<char*> CgiHandler::createEnvpArray(void) {
  std::vector<char*> vector{};
  for (auto& string : this->envps)
    vector.push_back(&string.front());
  return vector;
}

std::vector<char*> CgiHandler::createArgvArray(void) {
  std::vector<char*> vector{};
  for (auto& string : this->args)
    vector.push_back(&string.front());
  return vector;
}

void CgiHandler::executeCgiScript(void) {
  LOG_TRACE("Log entry from child process");
  std::vector<char*> envp = createEnvpArray();
  std::vector<char*> argv = createArgvArray();
  if (dup2(pipefd[Fd::Write], STDOUT_FILENO) == -1) {
    cgiError("Could not duplicate pipe fd");
  }
  if (execve(argv[0], argv.data(), envp.data()) == -1) {
    cgiError("Failed to execute CGI script");
  }
  closePipeFds();
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
    executeCgiScript();
  } else if (isParentProcess()) {
    if (dup2(pipefd[Fd::Read], sockfd) == -1) {
      cgiError("Could not duplicate pipe fd");
    }
  }
}

bool CgiHandler::isAccessable(void) const {
  struct stat s;
  if (!stat(cgi.c_str(), &s) && S_ISREG(s.st_mode) && !access(cgi.c_str(), X_OK)) {
    return true;
  }
  return false;
}

void CgiHandler::scriptLoader(void) {
  if (!isAccessable()) {
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
