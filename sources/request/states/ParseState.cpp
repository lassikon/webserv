#include <Client.hpp>
#include <ParseState.hpp>

void ParseState::execute(Client& client) {
  if (client.getClientState() != ClientState::READING) {
    return;
  }
  LOG_TRACE("Parsing request from client fd:", client.getFd());
  std::istringstream iBuf(client.getReadBuf()->data());
  if (client.getParsingState() == ParsingState::REQLINE) {
    parseRequestLine(client, iBuf);
  }
  if (client.getParsingState() == ParsingState::HEADER) {
    parseHeaders(client, iBuf);
  }
  if (client.getParsingState() == ParsingState::BODY) {
    parseBody(client, iBuf);
  }
  if (client.getParsingState() == ParsingState::DONE) {
    LOG_TRACE("Request received from client fd:", client.getFd());
  }
}

void ParseState::parseRequestLine(Client& client, std::istringstream& iBuf) {
  LOG_TRACE("Parsing request line");
  std::string requestLine;
  std::string reqMethod;
  std::string reqURI;
  std::string reqVersion;
  std::getline(iBuf, requestLine);
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

void ParseState::parseHeaders(Client& client, std::istringstream& iBuf) {
  LOG_TRACE("Parsing headers");
  std::string header;
  while (std::getline(iBuf, header)) {
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

void ParseState::parseBody(Client& client, std::istringstream& iBuf) {
  if (!isWithBody(client)) {
    client.setParsingState(ParsingState::DONE);
    client.setClientState(ClientState::PROCESSING);
    return;
  }
  LOG_TRACE("Parsing body");
  if (isChunked) {
    parseChunkedBody(client, iBuf);
  } else if (isWithContentLength(client)) {
    parseBodyWithContentLength(client, iBuf);
  } else if (isConnectionClose(client)) {
    parseBodyWithoutContentLength(client, iBuf);
    client.setClientState(ClientState::PROCESSING);
  } else {
    parseBodyWithoutContentLength(client, iBuf);
  }
}

void ParseState::parseBodyWithoutContentLength(Client& client, std::istringstream& iBuf) {
  LOG_WARN("Content-Length header not found. Using EOF");
  int bodySize = client.getReq().getBodySize();
  bodySize += client.getReadNBytes();
  client.getReq().setBodySize(bodySize);
  std::vector<char> bodyData(client.getReadNBytes());
  iBuf.read(bodyData.data(), client.getReadNBytes());
  std::vector<char> body = client.getReq().getBody();
  body.insert(body.end(), bodyData.begin(), bodyData.end());
  client.getReq().setBody(body);
  //client.setClientState(ClientState::PROCESSING);
}

void ParseState::parseBodyWithContentLength(Client& client, std::istringstream& iBuf) {
  int contentLength = std::stoi(client.getReq().getHeaders()["Content-Length"]);
  int bodySize = client.getReq().getBodySize();
  bodySize += contentLength;
  client.getReq().setBodySize(bodySize);
  std::vector<char> bodyData(contentLength);
  iBuf.read(bodyData.data(), contentLength);
  std::vector<char> body = client.getReq().getBody();
  body.insert(body.end(), bodyData.begin(), bodyData.end());
  client.getReq().setBody(body);
  client.setClientState(ClientState::PROCESSING);
}

void ParseState::parseChunkedBody(Client& client, std::istringstream& iBuf) {
  LOG_TRACE("Parsing chunked body");
  std::string chunkSizeHex;
  while (std::getline(iBuf, chunkSizeHex) && chunkSizeHex != "\r") {
    if (chunkSizeHex.back() == '\r') {
      chunkSizeHex.pop_back();
    }
    int bodySize = client.getReq().getBodySize();
    int chunkSize = std::stoi(chunkSizeHex, 0, 16);
    bodySize += chunkSize;
    client.getReq().setBodySize(bodySize);
    if (chunkSize == 0) {
      client.setClientState(ClientState::PROCESSING);
      break;
    }
    std::vector<char> chunkData(chunkSize);
    iBuf.read(chunkData.data(), chunkSize);
    std::vector<char> body = client.getReq().getBody();
    body.insert(body.end(), chunkData.begin(), chunkData.end());
    client.getReq().setBody(body);
    iBuf.ignore(2);  // remove \r\n
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
  return client.getReq().getMethod() == "POST" || client.getIsCgi();
}

bool ParseState::isWithContentLength(Client& client) {
  return client.getReq().getHeaders().find("Content-Length") != client.getReq().getHeaders().end();
}

bool ParseState::isConnectionClose(Client& client) {
  return client.getReq().getHeaders().find("Connection") != client.getReq().getHeaders().end() &&
         client.getReq().getHeaders()["Connection"] == "close";
}