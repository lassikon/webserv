#include <Client.hpp>
#include <NetworkException.hpp>
#include <ParseState.hpp>

void ParseState::execute(Client& client) {
  if (client.getClientState() != ClientState::READING || client.getReadBuf() == nullptr ||
      client.getParsingState() == ParsingState::IDLE) {
    return;
  }
  LOG_TRACE("Parsing request from client fd:", client.getFd());
  if (client.getParsingState() == ParsingState::REQLINE) {
    parseRequestLine(client);
  }
  if (client.getParsingState() == ParsingState::HEADER) {
    parseHeaders(client);
  }
  if (client.getParsingState() == ParsingState::VERIFY) {
    verifyRequest(client);  // verify request headers
  }
  if (client.getParsingState() == ParsingState::BODY) {
    parseBody(client);
  }
  for (auto& cgi : g_CgiParams) {
    if (cgi.clientFd == client.getFd() && cgi.isExited) {
      LOG_TRACE("Cgi process exited for client fd:", client.getFd());
      client.setCgiState(CgiState::DONE);
      client.setParsingState(ParsingState::DONE);
    }
  }
  if (client.getParsingState() == ParsingState::DONE) {
    LOG_TRACE("Request received from client fd:", client.getFd());
    client.setParsingState(ParsingState::IDLE);
    client.setClientState(ClientState::PROCESSING);
  }
}

void ParseState::parseRequestLine(Client& client) {
  LOG_TRACE("Parsing request line");
  std::string requestLine, reqMethod, reqURI, reqVersion;
  std::vector<char>& buffer = *client.getReadBuf();
  size_t& curr = client.getReadCurr();
  size_t& end = client.getReadEnd();
  Utility::getLineVectoStr(buffer, requestLine, curr, end);
  LOG_TRACE("Request line:", requestLine);
  std::istringstream iss(requestLine);
  iss >> reqMethod >> reqURI >> reqVersion;
  LOG_DEBUG("reqMethod:", reqMethod, "URI:", reqURI, "reqVersion:", reqVersion);
  if (reqMethod.empty() || reqURI.empty() || reqVersion.empty()) {
    throw httpBadRequest(client, "Invalid request line for client fd:", client.getFd());
  }
  client.getReq().setMethod(reqMethod);
  client.getReq().setReqURI(reqURI);
  client.getReq().setVersion(reqVersion);
  client.setParsingState(ParsingState::HEADER);
}

void ParseState::parseHeaders(Client& client) {
  LOG_TRACE("Parsing headers");
  std::string header;
  std::vector<char>& buffer = *client.getReadBuf();
  size_t& curr = client.getReadCurr();
  size_t& end = client.getReadEnd();
  while (Utility::getLineVectoStr(buffer, header, curr, end)) {
    if (isHeaderEnd(header)) {
      client.setParsingState(ParsingState::VERIFY);
      if (client.getCgiState() != CgiState::IDLE) {
        client.setParsingState(ParsingState::BODY);
      }
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
      if (key == "Content-Length" && client.getReq().getMethod() == "GET") {
        throw httpBadRequest(client, "Content-Length header found in GET request for client fd:", client.getFd());
      }
    }
  }
}

void ParseState::verifyRequest(Client& client) {
  LOG_TRACE("Verifying request");
  if (client.getReq().getVersion() != "HTTP/1.1") {
    throw httpVersion(client, "Unsupported HTTP version", client.getReq().getVersion(), "for client fd:", client.getFd());
  }
  client.getRes().setServerConfig(chooseServerConfig(client));  // choose server config
  buildPath(client);
  client.setParsingState(ParsingState::BODY);
}

void ParseState::parseBody(Client& client) {
  if (!isWithBody(client)) {
    LOG_TRACE("No body to parse");
    client.setParsingState(ParsingState::DONE);
    return;
  }
  LOG_TRACE("Parsing body");
  if (isChunked) {
    parseChunkedBody(client);
  } else if (isWithContentLength(client)) {
    parseBodyWithContentLength(client);
  } else {
    if (client.getCgiState() != CgiState::IDLE) {
      client.getReq().clearBody();
      parseBodyWithoutContentLength(client);
    } else {
      throw httpLength(client, "Content-Length header not found for client fd:", client.getFd());
    }
  }
}

void ParseState::parseBodyWithoutContentLength(Client& client) {
  LOG_TRACE("Content-Length header not found. Using EOF");
  std::vector<char>& buffer = *client.getReadBuf();
  size_t& curr = client.getReadCurr();
  size_t& end = client.getReadEnd();
  int bodySize = client.getReq().getBodySize();
  bodySize += end - curr;
  client.getReq().setBodySize(bodySize);
  LOG_DEBUG("distance(curr, end):", end - curr);
  std::vector<char> bodyData(end - curr + 1);
  bodyData.assign(buffer.begin() + curr, buffer.end());
  std::vector<char> body = client.getReq().getBody();
  body.insert(body.end(), bodyData.begin(), bodyData.end());
  client.getReq().setBody(body);
}

void ParseState::parseBodyWithContentLength(Client& client) {
  LOG_TRACE("Parsing body with content length");
  std::vector<char>& buffer = *client.getReadBuf();
  size_t& curr = client.getReadCurr();
  size_t& end = client.getReadEnd();
  size_t contentLength = std::stoi(client.getReq().getHeaders()["Content-Length"]);
  client.getReq().setBodySize(contentLength);
  std::vector<char> bodyData(contentLength);
  bodyData.assign(buffer.begin() + curr, buffer.end());
  client.getReq().setBody(bodyData);
  LOG_DEBUG("Read Bytes:", client.getReadNBytes());
  LOG_DEBUG("Content-Length:", contentLength);
  LOG_DEBUG("Distance:", end - curr);
  if (end - curr == contentLength) {
    client.setParsingState(ParsingState::DONE);
  }
}

void ParseState::parseChunkedBody(Client& client) {
  LOG_TRACE("Parsing chunked body");
  std::vector<char>& buffer = *client.getReadBuf();
  size_t& curr = client.getReadCurr();
  size_t& end = client.getReadEnd();
  std::string chunkSizeHex;
  size_t saveCurr = curr;
  size_t saveEnd = end;
  while (Utility::getLineVectoStr(buffer, chunkSizeHex, curr, end) && chunkSizeHex != "\r") {
    if (chunkSizeHex.back() == '\r') {
      chunkSizeHex.pop_back();
    }
    LOG_DEBUG("Chunk size hex:", chunkSizeHex);
    size_t bodySize = client.getReq().getBodySize();
    size_t chunkSize = std::stoi(chunkSizeHex, 0, 16);
    LOG_DEBUG("Chunk size:", chunkSize);
    bodySize += chunkSize;
    LOG_DEBUG("Body size:", bodySize);
    client.getReq().setBodySize(bodySize);
    if (chunkSize == 0) {
      client.setParsingState(ParsingState::DONE);
      isChunked = false;
      break;
    }
    // check for client body size limit
    if (bodySize > Utility::convertSizetoBytes(client.getRes().getServerConfig().clientBodySizeLimit)) {
      LOG_DEBUG("Client bodysize limit", client.getRes().getServerConfig().clientBodySizeLimit);
      throw httpPayload(client, "Client body size limit exceeded for client fd:", client.getFd());
    }
    if (curr + chunkSize + 2 > end) {
      LOG_WARN("Body not fully received. Waiting for more data");
      curr = saveCurr;
      end = saveEnd;
      break;
    }
    std::vector<char> chunkData(chunkSize);
    chunkData.assign(buffer.begin() + curr, buffer.begin() + curr + chunkSize);
    std::vector<char> body = client.getReq().getBody();
    body.insert(body.end(), chunkData.begin(), chunkData.end());
    client.getReq().setBody(body);
    curr += chunkSize + 2;  // skip CRLF
  }
}

// helper functions

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
  return header.empty() || header == "\r";
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

ServerConfig ParseState::chooseServerConfig(Client& client) {
  LOG_TRACE("Choosing server config for client fd:", client.getFd());
  for (auto& serverConfig : client.getServerConfigs()) {
    if (serverConfig->serverName == client.getReq().getHeaders()["Host"]) {
      return *serverConfig;
    }
  }
  return *(client.getServerConfigs().front());
}

void ParseState::buildPath(Client& client) {
  LOG_TRACE("Building path for client fd:", client.getFd());
  std::shared_ptr<ProcessTreeBuilder> ptb = std::make_shared<ProcessTreeBuilder>(client, client.getRes().getServerConfig());
  client.getRes().setReqURI(client.getReq().getReqURI());
  root = ptb->buildPathTree();
  root->process(client);
}
