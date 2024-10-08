#include <Client.hpp>
#include <NetworkException.hpp>
#include <ProcessState.hpp>

void ProcessState::execute(Client& client) {
  LOG_TRACE("Processing client fd:", client.getFd());
  if (client.getReadBuf() == nullptr) {
    client.setClientState(ClientState::DONE);
    return;
  }

  if (client.getFd() != client.getWriteFd()) {
    LOG_TRACE("Processing cgi body for client fd:", client.getFd());
    client.getRes().setResBody(client.getReq().getBody());
    client.getRes().makeBodytoCgi();
    client.setClientState(ClientState::SENDING);
  } else if (client.getFd() != client.getReadFd()) {
    LOG_TRACE("Processing CGI output for client fd:", client.getFd());
    processCgiOutput(client);
  } else {
    LOG_TRACE("Processing request for client fd:", client.getFd());
    processRequest(client);
  }
}

void ProcessState::processRequest(Client& client) {
  //client.getRes().setServerConfig(chooseServerConfig(client));  // choose server config
  //buildPath(client);
  if (client.getRes().getReqURI().find("/cgi-bin/") != std::string::npos) {
    client.getCgiHandler().executeRequest(client);
    if (client.getReq().getMethod() == "GET") {
      client.setClientState(ClientState::READING);
      client.setCgiState(CgiState::WRITING);
      client.getReadBuf()->clear();
      client.setReadBuf(nullptr);
      return;
    }
    if (client.getReq().getMethod() == "POST") {
      client.setCgiState(CgiState::READING);
      client.setClientState(ClientState::PROCESSING);
    }
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
  client.setClientState(ClientState::PREPARING);
}

void ProcessState::processCgiOutput(Client& client) {
  client.getRes().setResStatusCode(200);
  client.getRes().setResStatusMessage("OK");
  for (auto& header : client.getReq().getHeaders()) {
    client.getRes().addHeader(header.first, header.second);
  }
  client.getRes().addHeader("Connection", "keep-alive");
  client.getRes().addHeader("Content-Length", std::to_string(client.getReq().getBodySize()));
  std::vector<char> reqBody = client.getReq().getBody();
  client.getRes().setResBody(reqBody);
  client.setCgiState(CgiState::DONE);
  client.setClientState(ClientState::PREPARING);
}

/* ServerConfig ProcessState::chooseServerConfig(Client& client) {
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
} */