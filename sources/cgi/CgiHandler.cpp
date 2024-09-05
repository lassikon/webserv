#include <CgiHandler.hpp>

std::vector<pid_t> CgiHandler::pids;

CgiHandler::CgiHandler(void) {
  LOG_DEBUG(Utility::getConstructor(*this));
}

CgiHandler::~CgiHandler(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
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
      LOG_DEBUG("Terminating child pid:", pid);
      kill(pid, SIGTERM);
    }
  }
}

void CgiHandler::waitChildProcess(void) {
  waitpid(pid, &wstat, 0);
  if (WIFSIGNALED(wstat) != 0) {
    g_ExitStatus = (int)Error::Signal + WTERMSIG(wstat);
  } else {
    g_ExitStatus = WEXITSTATUS(wstat);
  }
}

void CgiHandler::setEnvParams(void) {}

void CgiHandler::executeCgiScript(void) {
  setEnvParams();
  if (dup2(pipefd[Fd::Write], STDOUT_FILENO) == -1) {
    cgiError("Could not duplicate pipe fd");
  }
}

bool CgiHandler::isParentProcess(void) const {
  return pid != 0 ? true : false;
}

bool CgiHandler::isChildProcess(void) const {
  return pid == 0 ? true : false;
}

const pid_t& CgiHandler::addNewProcessId(void) {
  pids.push_back(fork());
  return pids[pids.size() - 1];
}

void CgiHandler::createChildProcess(void) {
  pid = addNewProcessId();
  if (pid == -1) {
    cgiError("Could not create child process");
  } else if (isChildProcess()) {
    executeCgiScript();
  } else if (isParentProcess()) {
    if (dup2(pipefd[Fd::Read], sockfd) == -1) {
      cgiError("Could not duplicate pipe fd");
    }
  }
}

bool CgiHandler::validCgiScript(void) const {
  return true;
}

bool CgiHandler::validCgiAccess(void) const {
  return true;
}

void CgiHandler::cgiLoader(void) {
  if (!validCgiAccess()) {
    cgiError("Could not open CGI script");
  } else if (!validCgiScript()) {
    cgiError("Not a valid CGI script");
  } else if (pipe(pipefd) == -1) {
    cgiError("Could not create pipe");
  } else {
    createChildProcess();
    waitChildProcess();
  }
}

/* CGIMAP m; */
/* m.insert(std::make_pair("WHAT", "YES")); */
/* auto it = m.find("WHAT"); */
/* if (it != m.end()) */
/*   it->second = "NO"; */
/* std::cout << m.at("WHAT"); */
