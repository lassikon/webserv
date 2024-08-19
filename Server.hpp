#pragma once

class Server {
private:
  const float example = 42;

public:
  Server(void);
  Server(const Server &other);
  Server &operator=(const Server &other);
  ~Server(void);
};

