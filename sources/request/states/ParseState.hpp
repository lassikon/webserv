#pragma once

#include <IStateHandler.hpp>
#include <Logger.hpp>
#include <ProcessTree.hpp>
#include <ProcessTreeBuilder.hpp>

class Client;

class ParseState : public IStateHandler {

 private:
  bool isChunked = false;

 private:
  std::shared_ptr<ProcessTree> root;

 public:
  ParseState() = default;
  virtual ~ParseState() = default;
  void execute(Client& client) override;

 private:
  ServerConfig chooseServerConfig(Client& client);
  void buildPath(Client& client);
  void parseRequestLine(Client& client);
  void parseHeaders(Client& client);
  void verifyRequest(Client& client);
  void parseBody(Client& client);

 private:
  bool isHeaderEnd(std::string header);
  bool substrKeyAndValue(std::string header, std::string& key, std::string& value);
  bool isWithBody(Client& client);
  bool isWithContentLength(Client& client);
  bool isConnectionClose(Client& client);
  void parseChunkedBody(Client& client);
  void parseBodyWithContentLength(Client& client);
  void parseBodyWithoutContentLength(Client& client);
};