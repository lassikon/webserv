#include <CgiHandler.hpp>

std::vector<pid_t> CgiHandler::pids;

CgiHandler::CgiHandler(const Request& request) {
  LOG_DEBUG(Utility::getConstructor(*this));
  (void)request;
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
  /* } else if (execve(cgi.c_str(), NULL, NULL) == -1) { */
  /*   cgiError("Failed to execute CGI script"); */
  /* } */
  closePipeFds();
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

void CgiHandler::forkChildProcess(void) {
  LOG_TRACE("Forking new child process");
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

bool CgiHandler::isAccessable(void) const {
  return true;
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
  LOG_TRACE("Running new cgi instance");
  Exception::tryCatch(&CgiHandler::scriptLoader, this);
}

/* CGIMAP m; */
/* m.insert(std::make_pair("WHAT", "YES")); */
/* auto it = m.find("WHAT"); */
/* if (it != m.end()) */
/*   it->second = "NO"; */
/* std::cout << m.at("WHAT"); */

/* std::vector<std::string> strings{"one", "two", "three"}; */
/* std::vector<char*> cstrings; */
/* cstrings.reserve(strings.size()); */
/* for (int i = 0; i < strings.size(); ++i) */
/*   cstrings.push_back(const_cast<char*>(strings[i].c_str())); */
