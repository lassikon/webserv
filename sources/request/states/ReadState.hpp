#pragma once

#include <IStateHandler.hpp>
#include <Logger.hpp>
#include <vector>

class client;

class ReadState : public IStateHandler {
 public:
  ReadState() = default;
  virtual ~ReadState() = default;
  void execute(Client& client) override;

  private:
    bool isCgiOutput(Client& client, std::string buf);
    void isCRLF(Client& client);
    void handleIfEOF(Client& client);
    void handleIfCgiOutput(Client& client);
    void handleReadBuf(Client& client, std::vector<char> buffer, ssize_t nbytes);
};