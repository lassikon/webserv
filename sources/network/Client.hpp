#pragma once

class Client {
private:
  const float example = 42;

public:
  Client(void);
  Client(const Client &other);
  Client &operator=(const Client &other);
  ~Client(void);
};

