
#include <Client.hpp>
#include <PostHandler.hpp>
#include <NetworkException.hpp>

void PostHandler::getContentType(Client& client) {
  LOG_TRACE("Getting content type");
  contentType = client.getReq().getHeaders()["Content-Type"];
  if (contentType.empty()) {
    throw httpBadRequest(client, "Missing content type");
  }
  if (contentType.find("multipart/form-data") != std::string::npos) {
    contentType = "multipart/form-data";
  }
  LOG_DEBUG("Content type:", contentType);
}

void PostHandler::processFormUrlEncoded(Client& client) {
  LOG_TRACE("Processing application/x-www-form-urlencoded");
  std::string data(client.getReq().getBody().data(), client.getReq().getBody().size());
  std::istringstream iss(data);
  std::string pair;
  while (std::getline(iss, pair, '&')) {
    auto delimiterPos = pair.find('=');
    if (delimiterPos == std::string::npos) {
      throw httpBadRequest(client, "Invalid form data");
    }
    std::string key = pair.substr(0, delimiterPos);
    std::string value = pair.substr(delimiterPos + 1);
    formData[UrlEncoder::decode(key)] = UrlEncoder::decode(value);
  }
  LOG_DEBUG("Parsed form data:");  // For testing purposes
  for (const auto& [key, value] : formData) {
    details.push_back(value);
    LOG_DEBUG(key, ":", value);
  }
}

std::string PostHandler::extractBoundary(Client& client) {
  LOG_TRACE("Extracting boundary");
  std::string contentType = client.getReq().getHeaders()["Content-Type"];
  std::string boundary = contentType.substr(contentType.find("boundary=") + 9);
  LOG_DEBUG("Boundary:", boundary);
  return boundary;
}

std::vector<std::string> PostHandler::splitByBoundary(std::string data, std::string boundary) {
 LOG_TRACE("Splitting data by boundary");
    std::vector<std::string> parts;
    size_t pos = 0;

    if ((pos = data.find(boundary)) != std::string::npos) {
        data.erase(0, pos + boundary.length());
    }
    while ((pos = data.find(boundary)) != std::string::npos) {
        std::string part = data.substr(0, pos);
        part.erase(0, part.find_first_not_of("\r\n"));
        part.erase(part.find_last_not_of("\r\n") + 1);
        if (!part.empty()) {
            parts.push_back(part);
        }
        data.erase(0, pos + boundary.length());
    }
    return parts;
}

bool PostHandler::isFilePart(const std::string& part) {
  LOG_DEBUG("Checking if part is file", part);
 return part.find("filename") != std::string::npos;
}

std::string PostHandler::extractFileName(const std::string& part, Client& client) {
  LOG_TRACE("Extracting filename");
  size_t pos = part.find("filename=\"");
  if (pos == std::string::npos) {
    throw httpBadRequest(client, "Invalid file part");
  }
  pos += 10;
  size_t end = part.find("\"", pos);
  return part.substr(pos, end - pos);
}

std::string PostHandler::extractFileData(const std::string& part, Client& client) {
  LOG_TRACE("Extracting file data");
  size_t pos = part.find("\r\n\r\n");
  if (pos == std::string::npos) {
    throw httpBadRequest(client, "Invalid file part");
  }
  pos += 4;
  return part.substr(pos);
}

void PostHandler::processFilePart(Client& client, const std::string& part) {
  LOG_TRACE("Processing file part");
  std::string fileName = extractFileName(part, client);
  if (fileName.empty()){
    return;
  }
  std::string data = extractFileData(part, client);
  LOG_DEBUG("FileName:", fileName);
  // Save file to disk
  std::string path = client.getRes().getReqURI() + "/";
  LOG_DEBUG("Path:", path + fileName);
  // Check for permission to write to the file
  if (!access(path.c_str(), W_OK)) {
    LOG_DEBUG("Permission to write to file");
  } else {
    throw httpForbidden(client, "Permission denied to write to file");
  }
  std::ofstream file(path + fileName, std::ios::binary);
  if (!file.is_open()) {
    throw httpBadRequest(client, "Failed to open file");
  } else {
    LOG_TRACE("File opened successfully");
    file.write(data.data(), data.length());
    file.close();
    upload = true;
  }
}

void PostHandler::processFormData(const std::string& part, Client& client) {
  LOG_TRACE("Processing form data");
  std::istringstream iss(part);
  std::string pair;
  while (std::getline(iss, pair, '&')) {
    auto delimiterPos = pair.find('=');
    if (delimiterPos == std::string::npos) {
      throw httpBadRequest(client, "Invalid form data");
    }
    std::string key = pair.substr(0, delimiterPos);
    std::string value = pair.substr(delimiterPos + 1);
    formData[UrlEncoder::decode(key)] = UrlEncoder::decode(value);
  }
  LOG_DEBUG("Parsed form data:");
  for (const auto& [key, value] : formData) {
    details.push_back(value);
    LOG_DEBUG(key, ":", value);
  }
}

void PostHandler::processMultipartFormData(Client& client) {
  LOG_TRACE("Processing multipart/form-data");
  // check for body existence
  const auto& body = client.getReq().getBody();
  LOG_DEBUG("Body size:", body.size());
  if (body.empty()) {
    throw httpBadRequest(client, "Empty body");
  }
  std::string data(body.begin(), body.end());
  LOG_DEBUG("data length:", data.length());
  std::string boundary = extractBoundary(client);
  std::vector<std::string> parts = splitByBoundary(data, boundary);
  LOG_DEBUG("Parts size:", parts.size());
  for (const std::string& part : parts) {
    if (isFilePart(part)) {
      processFilePart(client, part);
    } else if (part == "--\r\n") {
      break;
    } else {
      processFormData(part, client);
    }
  }
}

void PostHandler::setResponse(Client& client) {
  LOG_TRACE("Populating response");
  client.getRes().setResStatusCode(200);
  client.getRes().setResStatusMessage("OK");
  client.getRes().addHeader("Content-Type", "text/html");
  std::string htmlResponse;
  if (upload) {
    // Set HTTP 303 status for redirection after a POST request
    client.getRes().setResStatusCode(303);
    client.getRes().setResStatusMessage("See Other");
    htmlResponse = "<html><body><h1>File uploaded. Redirecting...</h1></body></html>";
    client.getRes().addHeader("Location", "/upload/");
  }
  else {
    htmlResponse = "<html><h1>POST request processed</h1><body>";
    std::string data;
    for (const auto& detail : details) {
      LOG_DEBUG("Detail:", detail);
      data += detail + "<br>";
    }
    htmlResponse += "<p>Form Data:</p><pre>" + data + "</pre>";
    htmlResponse += "</body></html>";
  }
  client.getRes().addHeader("Content-Length", std::to_string(htmlResponse.size()));
  std::vector<char> responseBody(htmlResponse.begin(), htmlResponse.end());
  client.getRes().setResBody(responseBody);
}

void PostHandler::executeRequest(Client& client) {
  LOG_TRACE("Processing POST request for path:", client.getReq().getReqURI());
  client.setClientState(ClientState::PROCESSING);
  client.setParsingState(ParsingState::BODY);
  upload = false;

  getContentType(client);
  LOG_DEBUG("content type:", contentType);
  if (contentType == "application/x-www-form-urlencoded") {
    processFormUrlEncoded(client);
  } else if (contentType == "multipart/form-data") {
    processMultipartFormData(client);
  } else if (contentType == "text/plain") {
    std::string data(client.getReq().getBody().data(), client.getReq().getBody().size());
    details.push_back(data);
  } else {
    throw httpBadRequest(client, "Unsupported content type:", contentType);
  }
  setResponse(client);

  client.setClientState(ClientState::SENDING);
  client.setParsingState(ParsingState::DONE);
  LOG_INFO("Done processing POST request for path:", client.getReq().getReqURI());
}