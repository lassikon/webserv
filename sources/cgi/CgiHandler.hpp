#pragma once

class CgiHandler {
private:
  enum readWriteFds { READ, WRITE };

  int pipefd[2];
  int tempfd;

public:
  CgiHandler(void);
  ~CgiHandler(void);
};
