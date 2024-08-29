#include <Response.hpp>

Response::Response() {
    LOG_TRACE("response constructor called");
  transferEncodingChunked = false;
}

Response::~Response() {LOG_TRACE("response destructor called");}

void Response::makeResLine(std::ostringstream& oBuf) {
  LOG_TRACE("Making response line");
  oBuf << "HTTP/1.1 " << statusCode << " " << responseLine << "\r\n";
}

