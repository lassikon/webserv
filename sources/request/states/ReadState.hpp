#pragma once

#include <IStateHandler.hpp>
#include <Logger.hpp>

#include <memory>
#include <vector>

class client;

class ReadState : public IStateHandler {
 private:
  bool crlfFound = false;

 public:
  ReadState() = default;
  virtual ~ReadState() = default;
  void execute(Client& client) override;

 private:
  void ifCRLF(Client& client);
  void handleEOF(Client& client);
  void ifCgiOutput(Client& client);
  void handleReadBuf(Client& client, std::vector<char> buffer, ssize_t nbytes);
};
