#pragma once

#include <IStateHandler.hpp>
#include <Logger.hpp>

class Client;

class ParseState : public IStateHandler {

 private:
  bool isChunked = false;

 public:
  ParseState() = default;
  virtual ~ParseState() = default;
  void execute(Client& client) override;

 public:
  void parseRequestLine(Client& client, std::istringstream& iBuf);
  void parseHeaders(Client& client, std::istringstream& iBuf);
  void parseBody(Client& client, std::istringstream& iBuf);

 private:
  bool isHeaderEnd(std::string header);
  bool substrKeyAndValue(std::string header, std::string& key, std::string& value);
  bool isWithBody(Client& client);
  bool isWithContentLength(Client& client);
  bool isConnectionClose(Client& client);
  void parseChunkedBody(Client& client, std::istringstream& iBuf);
  void parseBodyWithContentLength(Client& client, std::istringstream& iBuf);
  void parseBodyWithoutContentLength(Client& client, std::istringstream& iBuf);
};