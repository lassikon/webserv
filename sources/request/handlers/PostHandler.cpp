
#include <Client.hpp>
#include <PostHandler.hpp>

void PostHandler::getContentType(Client& client) {
  LOG_INFO("Getting content type");
  contentType = client.getReq().getHeaders()["Content-Type"];
  if (contentType.empty()) {
    throw clientError("Content-Type header not found");
  }
  if (contentType.find("multipart/form-data") != std::string::npos) {
    // boundary = contentType.substr(contentType.find("boundary=") + 9);
    contentType = "multipart/form-data";
  }
  LOG_INFO("Content type:", contentType);
}

void PostHandler::processFormUrlEncoded(Client& client) {
  LOG_INFO("Processing application/x-www-form-urlencoded");
  std::string data(client.getReq().getBody().begin(), client.getReq().getBody().end());
  LOG_DEBUG("Raw data:\n", data);
  std::istringstream iss(data);
  std::string pair;

  while (std::getline(iss, pair, '&')) {
    auto delimiterPos = pair.find('=');
    if (delimiterPos == std::string::npos) {
      throw clientError("Invalid form data");
    }
    std::string key = pair.substr(0, delimiterPos);
    std::string value = pair.substr(delimiterPos + 1);
    formData[UrlEncoder::decode(key)] = UrlEncoder::decode(value);
  }
  LOG_DEBUG("Parsed form data:");  // For testing purposes
  for (const auto& [key, value] : formData) {
    LOG_DEBUG(key, ":", value);
  }
}

std::string PostHandler::extractBoundary(Client& client) {
  LOG_INFO("Extracting boundary");
  std::string contentType = client.getReq().getHeaders()["Content-Type"];
  std::string boundary = contentType.substr(contentType.find("boundary=") + 9);
  LOG_INFO("Boundary:", boundary);
  return boundary;
}

std::vector<std::string> PostHandler::splitByBoundary(std::string data, std::string boundary) {
  LOG_INFO("Splitting data by boundary");
  std::vector<std::string> parts;
  size_t pos = 0;
  while ((pos = data.find(boundary)) != std::string::npos) {
    parts.push_back(data.substr(0, pos));
    data.erase(0, pos + boundary.length());
  }
  return parts;
}

bool PostHandler::isFilePart(const std::string& part) {
  return part.find("filename") != std::string::npos;
}

std::string PostHandler::extractFileName(const std::string& part) {
  LOG_INFO("Extracting filename");
  size_t pos = part.find("filename=\"");
  if (pos == std::string::npos) {
    throw clientError("Invalid file part");
  }
  pos += 10;
  size_t end = part.find("\"", pos);
  return part.substr(pos, end - pos);
}

std::string PostHandler::extractFileData(const std::string& part) {
  LOG_INFO("Extracting file data");
  size_t pos = part.find("\r\n\r\n");
  if (pos == std::string::npos) {
    throw clientError("Invalid file part");
  }
  pos += 4;
  return part.substr(pos);
}

void PostHandler::processFilePart(const std::string& part) {
  LOG_INFO("Processing file part");
  std::string fileName = extractFileName(part);
  std::string data = extractFileData(part);
  LOG_DEBUG("FileName:", fileName);
  LOG_DEBUG("Data:\n", data);

  // Save file to disk
  std::ofstream file("uploads/" + fileName);
  file << data;
  file.close();
}

void PostHandler::processFormData(const std::string& part) {
  LOG_INFO("Processing form data");
  std::istringstream iss(part);
  std::string pair;
  while (std::getline(iss, pair, '&')) {
    auto delimiterPos = pair.find('=');
    if (delimiterPos == std::string::npos) {
      throw clientError("Invalid form data");
    }
    std::string key = pair.substr(0, delimiterPos);
    std::string value = pair.substr(delimiterPos + 1);
    formData[UrlEncoder::decode(key)] = UrlEncoder::decode(value);
  }
  LOG_DEBUG("Parsed form data:");
  for (const auto& [key, value] : formData) {
    LOG_DEBUG(key, ":", value);
  }
}

void PostHandler::processMultipartFormData(Client& client) {
  LOG_INFO("Processing multipart/form-data");
  // check for body existence
  const auto& body = client.getReq().getBody();
  LOG_DEBUG("Body size:", body.size());
  // for (size_t i = 0; i < body.size(); ++i) {  // Log body bytes
  //   LOG_DEBUG("Byte ", i, ": ", static_cast<int>(body[i]));
  // }
  std::string hexBody;
  for (const auto& c : body) {  // Convert body to hex for logging
    hexBody += std::to_string(c);
  }
  LOG_DEBUG("Body in hex:", hexBody);
  if (body.empty()) {
    throw clientError("Empty body");
  }
  std::string data(body.begin(), body.end());
  LOG_DEBUG("data length:", data.length());
  std::string boundary = extractBoundary(client);
  std::vector<std::string> parts = splitByBoundary(data, boundary);
  LOG_DEBUG("Parts size:", parts.size());
  for (const std::string& part : parts) {
    if (isFilePart(part)) {
      processFilePart(part);
    } else {
      processFormData(part);
    }
  }
}

void PostHandler::setResponse(Client& client) {
  LOG_INFO("Populating response");
  client.getRes().setResStatusCode(200);
  client.getRes().setResStatusMessage("OK");
  client.getRes().addHeader("Content-Type", "text/html");

  std::string htmlResponse = "<html><body><h1>POST request processed</h1><ul>";
  std::vector<char> responseBody(htmlResponse.begin(), htmlResponse.end());
  client.getRes().setResBody(responseBody);
}

void PostHandler::executeRequest(Client& client) {
  LOG_INFO("Processing POST request for path:", client.getReq().getReqURI());
  client.setClientState(ClientState::PROCESSING);
  client.setParsingState(ParsingState::BODY);

  getContentType(client);
  if (contentType == "application/x-www-form-urlencoded") {
    processFormUrlEncoded(client);
  } else if (contentType == "multipart/form-data") {
    processMultipartFormData(client);
  } else {
    throw clientError("Unsupported content type:", contentType);
  }
  setResponse(client);

  client.setClientState(ClientState::SENDING);
  client.setParsingState(ParsingState::DONE);
  LOG_INFO("Done processing POST request for path:", client.getReq().getReqURI());
}