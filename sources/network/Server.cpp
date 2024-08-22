#include <Server.hpp>

Server::Server(const std::string& port) : port(port), sockFD(-1) {}

Server::~Server(void) {}

void Server::run(void) {
  setup();

  // TODO: signal handling

  PollManager pollManager;
  pollManager.addFD(sockFD, POLLIN);

  while (true) {
    if (pollManager.pollFDsCount() == -1) {
      // error handling here
      break;
    }

    for (auto& pfd : pollManager.getPollFDs()) {
      if (pfd.revents & POLLIN) {
        if (pfd.fd == sockFD) {
          acceptConnection(pollManager);
        } else {
          handleClient(pollManager, pfd.fd);
        }
      }
    }
  }

  cleanup();
}

void Server::setup(void) {
  struct addrinfo hints;
  struct addrinfo* res;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(NULL, PORT, &hints, &res);
  sockFD = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  bind(sockFD, res->ai_addr, res->ai_addrlen);
  listen(sockFD, BACKLOG);
  freeaddrinfo(res);
}

void Server::acceptConnection(PollManager& pollManager) {
  struct sockaddr_storage their_addr {};
  socklen_t addr_size = sizeof(their_addr);
  int newFD = accept(sockFD, (struct sockaddr*)&their_addr, &addr_size);

  if (newFD != -1) {
    clients.emplace_back(newFD);
    pollManager.addFD(newFD, POLLIN);
    LOG_INFO("accepted client fd ", newFD);
  }
}

void Server::handleClient(PollManager& pollManager, int clientFD) {
  Client* client = nullptr;
  for (Client& c : clients) {
    if (c.getFD() == clientFD) {
      client = &c;
      break;
    }
  }

  if (client) {
    if (!client->receiveData()) {
      pollManager.removeFD(client->getFD());
      auto it = std::find(clients.begin(), clients.end(), *client);
      if (it != clients.end()) {
        clients.erase(it);
      }
    }
  }
}

void Server::cleanup(void) { close(sockFD); }