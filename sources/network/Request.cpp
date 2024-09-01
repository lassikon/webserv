#include <Client.hpp>
#include <Request.hpp>

void Request::parseRequestLine(Client* client, std::string& requestLine) {
  LOG_TRACE("Parsing request line");
  std::istringstream iss(requestLine);
  iss >> method >> reqURI >> version;
  client->setState(ClientState::READING_HEADER);
}

void Request::parseHeaders(Client* client, std::istringstream& iBuf) {
  LOG_TRACE("Parsing headers");
  std::string header;
  while (std::getline(iBuf, header)) {
    std::cout << "header: " << header << std::endl;
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
      std::string value = header.substr(pos + 1);
      value = Utility::trimWhitespaces(value);
      headers[key] = value;
      if (key == "Transfer-Encoding" && value == "chunked") {
        transferEncodingChunked = true;
      }
    }
  }
}

void Request::parseBody(Client* client, std::istringstream& iBuf) {
  if (method != "POST") {
    client->setState(ClientState::READING_DONE);
    return;
  }
  if (transferEncodingChunked) {
    LOG_TRACE("Parsing chunked body");
    std::string chunkSizeHex;
    while (std::getline(iBuf, chunkSizeHex) && chunkSizeHex != "\r") {
      if (chunkSizeHex.back() == '\r') {
        chunkSizeHex.pop_back();
      }
      int chunkSize = std::stoi(chunkSizeHex, 0, 16);
      if (chunkSize == 0) {
        client->setState(ClientState::READING_DONE);
        break;
      }
      std::vector<char> chunkData(chunkSize);
      iBuf.read(chunkData.data(), chunkSize);
      body.append(chunkData.data(), chunkSize);
      iBuf.ignore(2);  // remove \r\n
    }
  } else {
    LOG_TRACE("Parsing body");
    if (headers.find("Content-Length") == headers.end()) { // 400 bad request
      LOG_ERROR("Content-Length header not found");
      return;
    }
    int contentLength = std::stoi(headers["Content-Length"]);
    std::vector<char> bodyData(contentLength);
    iBuf.read(bodyData.data(), contentLength);
    body.append(bodyData.data(), contentLength);
    client->setState(ClientState::READING_DONE);
  }
}