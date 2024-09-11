#include <Client.hpp>
#include <Request.hpp>

Request::Request() {
  LOG_DEBUG(Utility::getConstructor(*this));
  transferEncodingChunked = false;
}

Request::~Request() {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}

void Request::parseRequestLine(Client* client, std::string& requestLine) {
  LOG_TRACE("Parsing request line");
  std::istringstream iss(requestLine);
  iss >> reqMethod >> reqURI >> reqVersion;
  LOG_DEBUG("reqMethod:", reqMethod, "URI:", reqURI, "reqVersion:", reqVersion);
  client->setState(ClientState::READING_HEADER);
}

void Request::parseHeaders(Client* client, std::istringstream& iBuf) {
  LOG_TRACE("Parsing headers");
  std::string header;
  while (std::getline(iBuf, header)) {
    if (header.find("\r\n\r") != std::string::npos || header.empty() ||
        header == "\r") {
      client->setState(ClientState::READING_BODY);
      break;
    }
    if (header.back() == '\r' && header.size() > 1) {
      header.pop_back();
    }
    auto pos = header.find(':');
    if (pos != std::string::npos) {
      std::string key = header.substr(0, pos);
      key = Utility::trimWhitespaces(key);
      std::string value = header.substr(pos + 1);
      value = Utility::trimWhitespaces(value);
      reqHeaders[key] = value;
      if (key == "Transfer-Encoding" && value == "chunked") {
        transferEncodingChunked = true;
      }
    }
  }
}

void Request::parseBody(Client* client, std::istringstream& iBuf, int nbytes) {
  if (reqMethod != "POST") {
    client->setState(ClientState::READING_DONE);
    return;
  }
  if (transferEncodingChunked) {
    parseChunkedBody(client, iBuf);
  } else {
    LOG_TRACE("Parsing body");
    if (reqHeaders.find("Content-Length") !=
        reqHeaders.end()) {  // 400 bad request
      int contentLength = std::stoi(reqHeaders["Content-Length"]);
      reqBodySize += contentLength;
      std::vector<char> bodyData(contentLength);
      iBuf.read(bodyData.data(), contentLength);
      reqBody.insert(reqBody.end(), bodyData.begin(), bodyData.end());
      client->setState(ClientState::READING_DONE);
    } else if (reqHeaders.find("Connection") != reqHeaders.end() &&
               reqHeaders["Connection"] == "close") {
      client->setState(ClientState::READING_DONE);
    } else {
      LOG_ERROR("Content-Length header not found");
      reqBodySize = nbytes;
      std::vector<char> bodyData(nbytes);
      iBuf.read(bodyData.data(), nbytes);
      reqBody.insert(reqBody.end(), bodyData.begin(), bodyData.end());
    }
  }
}

void Request::parseChunkedBody(Client* client, std::istringstream& iBuf) {
  LOG_TRACE("Parsing chunked body");
  std::string chunkSizeHex;
  while (std::getline(iBuf, chunkSizeHex) && chunkSizeHex != "\r") {
    if (chunkSizeHex.back() == '\r') {
      chunkSizeHex.pop_back();
    }
    int chunkSize = std::stoi(chunkSizeHex, 0, 16);
    reqBodySize += chunkSize;
    if (chunkSize == 0) {
      client->setState(ClientState::READING_DONE);
      break;
    }
    std::vector<char> chunkData(chunkSize);
    iBuf.read(chunkData.data(), chunkSize);
    reqBody.insert(reqBody.end(), chunkData.begin(), chunkData.end());
    iBuf.ignore(2);  // remove \r\n
  }
}

std::string Request::getMethod(void) const {
  return reqMethod;
}

std::string Request::getReqURI(void) const {
  return reqURI;
}

std::string Request::getVersion(void) const {
  return reqVersion;
}

size_t Request::getBodySize(void) const {
  return reqBodySize;
}

std::map<std::string, std::string> Request::getHeaders(void) const {
  return reqHeaders;
}

std::vector<char> Request::getBody(void) const {
  return reqBody;
}

bool Request::isTransferEncodingChunked(void) const {
  return transferEncodingChunked;
}