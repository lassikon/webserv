#include <Client.hpp>
#include <ProcessState.hpp>

void ProcessState::execute(Client& client) {
  if (client.getReadBuf() == nullptr) {
    client.setClientState(ClientState::DONE);
    return;
  }
  LOG_TRACE("Processing request for client fd:", client.getFd());
  if (client.getIsCgi()) {
    processCgiOutput(client);
  } else {
    processRequest(client);
  }
}

void ProcessState::processRequest(Client& client) {
  client.getRes().setServerConfig(chooseServerConfig(client));  // choose server config
  buildPath(client);
  if (client.getRes().getReqURI().find("/cgi-bin/") != std::string::npos) {
    client.setIsCgi(true);
    client.getCgiHandler().executeRequest(client);
    client.setClientState(ClientState::READING);
    return;
  } else if (client.getReq().getMethod() == "GET") {
    client.getGetHandler().executeRequest(client);
  } else if (client.getReq().getMethod() == "POST") {
    client.getPostHandler().executeRequest(client);
  } else if (client.getReq().getMethod() == "DELETE") {
    client.getDeleteHandler().executeRequest(client);
  } else {
    LOG_ERROR("Unsupported method in client:", client.getFd());
  }
  client.setClientState(ClientState::SENDING);
}

void ProcessState::processCgiOutput(Client& client) {
  client.getRes().setResStatusCode(200);
  client.getRes().setResStatusMessage("OK");
  if (isWithStatusCode(client)) {
    std::string statusLine = client.getReq().getHeaders()["Status"];
    client.getRes().setResStatusCode(std::stoi(statusLine.substr(0, 3)));
    client.getRes().setResStatusMessage(statusLine.substr(4));
  }
  for (auto& header : client.getReq().getHeaders()) {
    if (header.first != "Status") {
      client.getRes().addHeader(header.first, header.second);
    }
  }
  client.getRes().addHeader("Connection", "keep-alive");
  client.getRes().addHeader("Content-Length", std::to_string(client.getReq().getBodySize()));
  std::vector<char> reqBody = client.getReq().getBody();
  client.getRes().setResBody(reqBody);
  client.setClientState(ClientState::SENDING);
}

ServerConfig ProcessState::chooseServerConfig(Client& client) {
  LOG_TRACE("Choosing server config for client fd:", client.getFd());
  for (auto& serverConfig : client.getServerConfigs()) {
    if (serverConfig->serverName == client.getReq().getHeaders()["Host"]) {
      return *serverConfig;
    }
  }
  return *(client.getServerConfigs().front());
}

void ProcessState::buildPath(Client& client) {
  LOG_TRACE("Building path for client fd:", client.getFd());
  std::shared_ptr<ProcessTreeBuilder> ptb =
    std::make_shared<ProcessTreeBuilder>(client, client.getRes().getServerConfig());
  client.getRes().setReqURI(client.getReq().getReqURI());
  root = ptb->buildPathTree();
  root->process(client);
}

bool ProcessState::isWithStatusCode(Client& client) {
  return client.getReq().getHeaders().find("Status") != client.getReq().getHeaders().end();
}