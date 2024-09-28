#include <Client.hpp>
#include <ParseState.hpp>

void ParseState::execute(Client& client) {
  if (client.getClientState() != ClientState::READING || client.getReadBuf() == nullptr) {
    return;
  }
  LOG_TRACE("Parsing request from client fd:", client.getFd());
  if (client.getParsingState() == ParsingState::REQLINE) {
    parseRequestLine(client, client.getReadIt(), client.getReadEnd());
  }
  if (client.getParsingState() == ParsingState::HEADER) {
    parseHeaders(client, client.getReadIt(), client.getReadEnd());
  }
  if (client.getParsingState() == ParsingState::BODY) {
    parseBody(client, client.getReadIt(), client.getReadEnd());
  }
  for (auto& cgi : g_CgiParams) {
    if (cgi.clientFd == client.getFd() && cgi.isExited) {
      client.setCgiState(CgiState::DONE);
      client.setParsingState(ParsingState::DONE);
    }
  }
  if (client.getParsingState() == ParsingState::DONE) {
    LOG_TRACE("Request received from client fd:", client.getFd());
    client.setClientState(ClientState::PROCESSING);
  }
}

void ParseState::parseRequestLine(Client& client, std::vector<char>::const_iterator& it,
                                  std::vector<char>::const_iterator& end) {
  LOG_TRACE("Parsing request line");
  std::string requestLine;
  std::string reqMethod;
  std::string reqURI;
  std::string reqVersion;
  Utility::getLineVectoStr(requestLine, it, end);
  LOG_TRACE("Request line:", requestLine);
  std::istringstream iss(requestLine);
  iss >> reqMethod >> reqURI >> reqVersion;
  LOG_DEBUG("reqMethod:", reqMethod, "URI:", reqURI, "reqVersion:", reqVersion);
  if (reqMethod.empty() || reqURI.empty() || reqVersion.empty()) {
    LOG_ERROR("Invalid request line");
    // throw exception bad request
    return;
  }
  client.getReq().setMethod(reqMethod);
  client.getReq().setReqURI(reqURI);
  client.getReq().setVersion(reqVersion);
  client.setParsingState(ParsingState::HEADER);
}

void ParseState::parseHeaders(Client& client, std::vector<char>::const_iterator& it,
                              std::vector<char>::const_iterator& end) {
  LOG_TRACE("Parsing headers");
  std::string header;
  while (Utility::getLineVectoStr(header, it, end)) {
    if (isHeaderEnd(header)) {
      client.setParsingState(ParsingState::BODY);
      break;
    }
    if (header.back() == '\r' && header.size() > 1) {
      header.pop_back();
    }
    std::string key;
    std::string value;
    if (substrKeyAndValue(header, key, value)) {
      client.getReq().getHeaders()[key] = value;
      if (key == "Transfer-Encoding" && value == "chunked") {
        isChunked = true;
      }
    }
  }
}

void ParseState::parseBody(Client& client, std::vector<char>::const_iterator& it,
                           std::vector<char>::const_iterator& end) {
  if (!isWithBody(client)) {
    client.setParsingState(ParsingState::DONE);
    return;
  }
  LOG_TRACE("Parsing body");
  if (isChunked) {
    parseChunkedBody(client, it, end);
  } else if (isWithContentLength(client)) {
    parseBodyWithContentLength(client, it, end);
  } else {
    parseBodyWithoutContentLength(client, it, end);
  }
}

void ParseState::parseBodyWithoutContentLength(Client& client, std::vector<char>::const_iterator& it,
                                               std::vector<char>::const_iterator& end) {
  LOG_WARN("Content-Length header not found. Using EOF");
  int bodySize = client.getReq().getBodySize();
  bodySize += std::distance(it, end);
  client.getReq().setBodySize(bodySize);
  LOG_DEBUG("std::distance(it, end):", std::distance(it, end));
  std::vector<char> bodyData(std::distance(it, end));
  bodyData.assign(it, end);
  std::vector<char> body = client.getReq().getBody();
  body.insert(body.end(), bodyData.begin(), bodyData.end());
  client.getReq().setBody(body);
}

void ParseState::parseBodyWithContentLength(Client& client, std::vector<char>::const_iterator& it,
                                            std::vector<char>::const_iterator& end) {
  LOG_DEBUG("Parsing body with content length");
  int contentLength = std::stoi(client.getReq().getHeaders()["Content-Length"]);
  client.getReq().setBodySize(contentLength);
  std::vector<char> bodyData(contentLength);
  bodyData.assign(it, end);
  client.getReq().setBody(bodyData);
  LOG_INFO("Read Bytes:", client.getReadNBytes());
  LOG_INFO("Content-Length:", contentLength);
  LOG_INFO("diff:", std::distance(it, end));
  if (std::distance(it, end) == contentLength) {
    client.setParsingState(ParsingState::DONE);
  }
}

void ParseState::parseChunkedBody(Client& client, std::vector<char>::const_iterator& it,
                                  std::vector<char>::const_iterator& end) {
  LOG_TRACE("Parsing chunked body");
  std::string chunkSizeHex;
  while (Utility::getLineVectoStr(chunkSizeHex, it, end) && chunkSizeHex != "\r") {
    if (chunkSizeHex.back() == '\r') {
      chunkSizeHex.pop_back();
    }
    LOG_DEBUG("Chunk size:", chunkSizeHex);
    int bodySize = client.getReq().getBodySize();
    int chunkSize = std::stoi(chunkSizeHex, 0, 16);
    bodySize += chunkSize;
    client.getReq().setBodySize(bodySize);
    if (chunkSize == 0) {
      client.setParsingState(ParsingState::DONE);
      break;
    }
    std::vector<char> chunkData(chunkSize);
    chunkData.assign(it, it + chunkSize);
    std::vector<char> body = client.getReq().getBody();
    body.insert(body.end(), chunkData.begin(), chunkData.end());
    client.getReq().setBody(body);
    it += chunkSize + 2;  // skip CRLF
  }
}

//helper functions

bool ParseState::substrKeyAndValue(std::string header, std::string& key, std::string& value) {
  auto pos = header.find(':');
  if (pos != std::string::npos) {
    key = header.substr(0, pos);
    key = Utility::trimWhitespaces(key);
    value = header.substr(pos + 1);
    value = Utility::trimWhitespaces(value);
    return true;
  }
  return false;
}

bool ParseState::isHeaderEnd(std::string header) {
  return header.find("\r\n\r") != std::string::npos || header.empty() || header == "\r";
}

bool ParseState::isWithBody(Client& client) {
  return client.getReq().getMethod() == "POST" || client.getReadFd() != client.getFd();
}

bool ParseState::isWithContentLength(Client& client) {
  return client.getReq().getHeaders().find("Content-Length") != client.getReq().getHeaders().end();
}

bool ParseState::isConnectionClose(Client& client) {
  return client.getReq().getHeaders().find("Connection") != client.getReq().getHeaders().end() &&
         client.getReq().getHeaders()["Connection"] == "close";
}
