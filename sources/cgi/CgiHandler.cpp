#include <CgiHandler.hpp>

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

void CgiHandler::setEnvParams(void) {}

void CgiHandler::executeCgiScript(void) {
  setEnvParams();
  if (dup2(pipefd[Fd::Write], STDOUT_FILENO) == -1) {
    THROW(Error::Cgi, "Could not duplicate pipe fd");
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

void CgiHandler::createChildProcess(void) {
  pid = fork();
  if (pid == -1) {
    THROW(Error::Cgi, "Could not create child process")
  } else if (pid == 0) {  // child process
    executeCgiScript();
  } else if (pid != 0) {  // parent process
    if (dup2(pipefd[Fd::Read], tempfd) == -1) {
      THROW(Error::Cgi, "Could not duplicate pipe fd");
    }
    closePipeFds();
  }
}

bool CgiHandler::validCgiFile(void) {
  return true;
}

void CgiHandler::cgiLoader(void) {
  if (!validCgiFile()) {
    THROW(Error::Cgi, "Could not open script");
  } else if (pipe(pipefd)) {
    THROW(Error::Cgi, "Could not create pipe");
  }
  createChildProcess();
  waitChildProcess();
}

/* CGIMAP m; */
/* m.insert(std::make_pair("WHAT", "YES")); */
/* auto it = m.find("WHAT"); */
/* if (it != m.end()) */
/*   it->second = "NO"; */
/* std::cout << m.at("WHAT"); */
