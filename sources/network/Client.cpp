#include <Client.hpp>

Client::Client(int socketFd) : fd(socketFd) {
  LOG_DEBUG("Client constructor called");
  state = ClientState::READING_REQLINE;
}

Client::~Client(void) {
  LOG_DEBUG("Client destructor called");
  cleanupClient();
}

bool Client::operator==(const Client& other) const { return fd == other.fd; }

bool Client::receiveData(void) {
  char buf[4096] = {0};
  int nbytes = recv(fd, buf, sizeof(buf), 0);
  if (nbytes == -1 && errno != EWOULDBLOCK && errno != EAGAIN) {
    LOG_ERROR("Failed to recv() from fd:", fd);
    // throw exception
  } else if (nbytes == 0) {  // Connection closed
    LOG_DEBUG("Connection closed for client fd:", fd);
    return false;
  } else {
    LOG_INFO("Receiving data from client fd", fd, ", buffer:", buf);
    // Echo data back to the client
    if (send(fd, buf, nbytes, 0) == -1) {
      LOG_ERROR("Send() failed with fd:", fd);
      // throw exception
      return false;
    }
  }
  return true;
}

// bool Client::receiveData(void) {
//   char buf[4096] = {0};
//   int nbytes = recv(fd, buf, sizeof(buf), 0);
//   if (nbytes == -1 && errno != EWOULDBLOCK && errno != EAGAIN) {
//     LOG_ERROR("Failed to recv() from fd:", fd);
//     // throw exception
//     return false;
//   } else if (nbytes == 0) {  // Connection closed
//     LOG_DEBUG("Connection closed for client fd:", fd);
//     return false;
//   } else {
//     LOG_DEBUG("Receiving data from client fd", fd, ", buffer:", buf);
//     // Echo data back to the client
//     std::string bufStr(buf);
//     processRequest(bufStr);
//     /*     std::string content = "<html><body><h1>Hello,
//        World!</h1></body></html>";

//         std::ostringstream oss;
//         oss << "HTTP/1.1 200 OK\r\n";
//         oss << "Cache-Control: no-cache, private\r\n";
//         oss << "Content-Type: text/html\r\n";
//         oss << "Content-Length: " << content.size() << "\r\n";
//         oss << "\r\n";
//         oss << content;
//         std::string response = oss.str();

//         if (send(fd, response.c_str(), response.size() + 1, 0) == -1) {
//           LOG_ERROR("Send() failed with fd:", fd);
//           // throw exception
//           return false;
//         } */
//   }
//   return true;
// }

void Client::processRequest(std::string& buf) {
  LOG_DEBUG("Processing request from client fd:", fd);
  std::istringstream iBuf(buf);
  if (state == ClientState::READING_REQLINE) {
    std::string requestLine;
    std::getline(iBuf, requestLine);
    parseRequestLine(req, requestLine);
  }
  if (state == ClientState::READING_HEADER) {
    parseHeaders(req, iBuf);
  }
  std::cout << "method: " << req.method << std::endl;
  std::cout << static_cast<int>(state) << std::endl;
  if (req.method == "POST" && state == ClientState::READING_BODY) {
    parseBody(req, iBuf);
  } else if (req.method == "GET" || req.method == "DELETE") {
    state = ClientState::READING_DONE;
  }
  if (state == ClientState::READING_DONE) {
    std::cout << "Request received from client fd:" << fd << std::endl;
    handleRequest(req);
  }
}

void Client::parseRequestLine(HttpReq& req, std::string& requestLine) {
  LOG_TRACE("Parsing request line");
  std::istringstream iss(requestLine);
  iss >> req.method >> req.path >> req.version;
  state = ClientState::READING_HEADER;
}

void Client::parseHeaders(HttpReq& req, std::istringstream& iBuf) {
  LOG_TRACE("Parsing headers");
  std::string header;
  while (std::getline(iBuf, header) && header != "\r") {
    std::cout << "header: " << header << std::endl;
    if (header.find("\r\n\r") != std::string::npos) {
      state = ClientState::READING_BODY;
      break;
    }
    if (header.back() == '\r') {
      header.pop_back();
    }
    auto pos = header.find(':');
    if (pos != std::string::npos) {
      std::string key = header.substr(0, pos);
      std::string value = header.substr(pos + 1);
      value = Utility::trimWhitespaces(value);
      req.headers[key] = value;
      if (key == "Transfer-Encoding" && value == "chunked") {
        req.transferEncodingChunked = true;
      }
    }
  }
}

void Client::parseBody(HttpReq& req, std::istringstream& iBuf) {
  if (req.transferEncodingChunked) {
    LOG_TRACE("Parsing chunked body");
    std::string chunkSizeHex;
    while (std::getline(iBuf, chunkSizeHex) && chunkSizeHex != "\r") {
      if (chunkSizeHex.back() == '\r') {
        chunkSizeHex.pop_back();
      }
      int chunkSize = std::stoi(chunkSizeHex, 0, 16);
      std::cout << "chunk size: " << chunkSize << std::endl;
      if (chunkSize == 0) {
        state = ClientState::READING_DONE;
        break;
      }

      std::vector<char> chunkData(chunkSize);
      iBuf.read(chunkData.data(), chunkSize);
      req.body.append(chunkData.data(), chunkSize);
      iBuf.ignore(2);  // remove \r\n
    }
  } else {
    LOG_TRACE("Parsing body");
    int contentLength = std::stoi(req.headers["Content-Length"]);
    std::vector<char> bodyData(contentLength);
    iBuf.read(bodyData.data(), contentLength);
    req.body.append(bodyData.data(), contentLength);
    state = ClientState::READING_DONE;
  }
}

void Client::handleRequest(HttpReq& req) {
  LOG_TRACE("Handling request from client fd:", fd);
  if (req.method == "GET") {
    LOG_INFO("GET request for path:", req.path);
    LOG_INFO("Request body:", req.body);
    LOG_INFO("Request headers:");
    for (auto& header : req.headers) {
      LOG_INFO(header.first, ":", header.second);
    }
    // sendResponse(req);
  } else if (req.method == "POST") {
    LOG_INFO("POST request for path:", req.path);
    LOG_INFO("Request body:", req.body);
    LOG_INFO("Request headers:");
    for (auto& header : req.headers) {
      LOG_INFO(header.first, ":", header.second);
    }
  } else if (req.method == "DELETE") {
    LOG_INFO("DELETE request for path:", req.path);
    LOG_INFO("Request body:", req.body);
    LOG_INFO("Request headers:");
    for (auto& header : req.headers) {
      LOG_INFO(header.first, ":", header.second);
    }
  } else {
    LOG_ERROR("Unsupported method:", req.method);
  }
}

void Client::setFd(int newFd) {
  if (fd != newFd) {
    LOG_DEBUG("Changing fd from:", fd, "to:", newFd);
    cleanupClient();
    fd = newFd;
  }
}

void Client::cleanupClient(void) {
  if (fd > 0) {
    LOG_DEBUG("cleanupClient() closing fd:", fd);
    if (close(fd) == -1) {
      LOG_ERROR("Failed to close fd:", fd);
      // throw exception
    }
    fd = -1;  // Mark as closed
  }
}
