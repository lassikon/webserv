#include <Client.hpp>
#include <Logger.hpp>
#include <PostHandler.hpp>
#include <UrlEncoder.hpp>

void PostHandler::getContentType(Client& client) {
  LOG_INFO("Getting content type");
  contentType = client.getReq().getHeaders()["Content-Type"];
  if (contentType.empty()) {
    throw clientError("Content-Type header not found");
  }
  LOG_INFO("Content type:", contentType);
}

void PostHandler::processFormUrlEncoded(Client& client) {
  LOG_INFO("Processing application/x-www-form-urlencoded");
  std::string data(client.getReq().getBody().begin(), client.getReq().getBody().end());
  LOG_DEBUG("DATA:\n", data);
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
  LOG_DEBUG("Parsed form data:");
  for (const auto& [key, value] : formData) {
    LOG_DEBUG(key, ":", value);
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
    LOG_INFO("Processing multipart/form-data");
  } else {
    throw clientError("Unsupported content type:", contentType);
  }
  setResponse(client);

  client.setClientState(ClientState::SENDING);
  client.setParsingState(ParsingState::DONE);
  LOG_INFO("Done processing POST request for path:", client.getReq().getReqURI());
}