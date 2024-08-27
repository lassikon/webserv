#include <Client.hpp>

Client::Client(int socketFd) : fd(socketFd) {
  LOG_DEBUG("Client constructor called");
}

Client::~Client(void) {
  LOG_DEBUG("Client destructor called");
  if (fd > 0) {
    LOG_DEBUG("Client destructor closing fd:", fd);
    if (close(fd) == -1) {
      LOG_ERROR("Failed to close fd:", fd);
      // throw exception
    }
  }
}

bool Client::operator==(const Client& other) const { return fd == other.fd; }

bool Client::receiveData(void) {
  char buf[4096] = {0};
  int nbytes = recv(fd, buf, sizeof(buf), 0);
  if (nbytes == -1) {
    LOG_ERROR("Failed to recv() from fd:", fd);
    // throw exception
  } else if (nbytes == 0) {  //|| (errno != EWOULDBLOCK && errno != EAGAIN)) {
    return false;            // Connection closed or error
  } else {
    LOG_INFO("Receiving data from client fd", fd, ", buffer:", buf);
    // Echo data back to the client

    std::string content = "<html><body><h1>Hello, World!</h1></body></html>";

    std::ostringstream oss;
    oss << "HTTP/1.1 200 OK\r\n";
    oss << "Cache-Control: no-cache, private\r\n";
    oss << "Content-Type: text/html\r\n";
    oss << "Content-Length: " << content.size() << "\r\n";
    oss << "\r\n";
    oss << content;
    std::string response = oss.str();

    if (send(fd, response.c_str(), response.size() + 1, 0) == -1) {
      LOG_ERROR("Send() failed with fd:", fd);
      // throw exception
    }
  }
  return true;
}

void Client::processRequest(std::string& buf) {
  LOG_INFO("Processing request from client fd:", fd, ", buffer:", buf);
  HttpReq req;
  std::istringstream iBuf(buf);
  std::string requestLine;
  std::getline(iBuf, requestLine);
  // parse request line GET, POST, DELETE
  parseRequestLine(req, requestLine);
  // Parse header content length or transfer encoding
  parseHeaders(req, iBuf);
  // Parse body
  parseBody(req, iBuf);
  // Process request
  handleRequest(req);
  // Send response
}

void Client::parseRequestLine(HttpReq& req, std::string& requestLine) {
  std::istringstream iss(requestLine);
  iss >> req.method >> req.path >> req.version;
}

void Client::parseHeaders(HttpReq& req, std::istringstream& iBuf) {
  std::string header;
  while (std::getline(iBuf, header) && header != "\r") {
    if (header.back() == '\r') {
      header.pop_back();
    }
    if (header.find("\r\n\r\n") != std::string::npos) {
      break;
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
    std::string chunkSizeHex;
    while (std::getline(iBuf, chunkSizeHex) && chunkSizeHex != "\r") {
      if (chunkSizeHex.back() == '\r') {
        chunkSizeHex.pop_back();
      }
      int chunkSize = std::stoi(chunkSizeHex, 0, 16);
      if (chunkSize == 0) {
        break;
      }

      std::vector<char> chunkData(chunkSize);
      iBuf.read(chunkData.data(), chunkSize);
      req.body.append(chunkData.data(), chunkSize);
      iBuf.ignore(2);  // remove \r\n
    }
  } else {
    int contentLength = std::stoi(req.headers["Content-Length"]);
    std::vector<char> bodyData(contentLength);
    iBuf.read(bodyData.data(), contentLength);
    req.body.append(bodyData.data(), contentLength);
  }
}

void Client::handleRequest(HttpReq& req) {
  LOG_INFO("Handling request from client fd:", fd);
  if (req.method == "GET") {
    LOG_INFO("GET request for path:", req.path);
    sendResponse(req, "GET");
  } else if (req.method == "POST") {
    LOG_INFO("POST request for path:", req.path);
  } else if (req.method == "DELETE") {
    LOG_INFO("DELETE request for path:", req.path);
  } else {
    LOG_ERROR("Unsupported method:", req.method);
  }
}

int Client::getFd(void) const { return fd; }

void Client::setFd(int fd) {
  if (this->fd != fd) {
    LOG_DEBUG("setFd closing fd:", fd);
    if (close(this->fd) == -1) {
      LOG_ERROR("Failed to close fd:", this->fd);
      // throw exception
    }
    this->fd = fd;
  }
}