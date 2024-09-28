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
  void parseRequestLine(Client& client, std::vector<char>::const_iterator& it,
                        std::vector<char>::const_iterator& end);
  void parseHeaders(Client& client, std::vector<char>::const_iterator& it,
                    std::vector<char>::const_iterator& end);
  void parseBody(Client& client, std::vector<char>::const_iterator& it,
                 std::vector<char>::const_iterator& end);

 private:
  bool isHeaderEnd(std::string header);
  bool substrKeyAndValue(std::string header, std::string& key, std::string& value);
  bool isWithBody(Client& client);
  bool isWithContentLength(Client& client);
  bool isConnectionClose(Client& client);
  void parseChunkedBody(Client& client, std::vector<char>::const_iterator& it,
                        std::vector<char>::const_iterator& end);
  void parseBodyWithContentLength(Client& client, std::vector<char>::const_iterator& it,
                                  std::vector<char>::const_iterator& end);
  void parseBodyWithoutContentLength(Client& client, std::vector<char>::const_iterator& it,
                                     std::vector<char>::const_iterator& end);
};