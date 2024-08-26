#include <Server.hpp>

Server::Server(const std::string& port) : port(port), sockFd(-1) {
  LOG_DEBUG("Server constructor called");
}

Server::~Server(void) { LOG_DEBUG("Server destructor called"); }

void Server::run(void) {
  setup();

  // TODO: signal handling

  PollManager pollManager;
  pollManager.addFd(sockFd, POLLIN);

  while (true) {
    if (pollManager.pollFdsCount() == -1) {
      // throw exception
      break;
    }

    for (auto& pfd : pollManager.getPollFds()) {
      if (pfd.revents & POLLIN) {
        if (pfd.fd == sockFd) {
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
  sockFd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  bind(sockFd, res->ai_addr, res->ai_addrlen);
  listen(sockFd, BACKLOG);
  freeaddrinfo(res);
}

void Server::acceptConnection(PollManager& pollManager) {
  struct sockaddr_storage their_addr {};
  socklen_t addr_size = sizeof(their_addr);
  int newFD = accept(sockFd, (struct sockaddr*)&their_addr, &addr_size);

  if (newFD != -1) {
    clients.emplace_back(newFD);
    pollManager.addFd(newFD, POLLIN);
    LOG_TRACE("Accepted client fd", newFD);
  }
}

void Server::handleClient(PollManager& pollManager, int clientFd) {
  auto it = std::find_if(clients.begin(), clients.end(),
                         [clientFd](Client& c) { return c.getFd() == clientFd; });
  if (it != clients.end()) {
    if (!it->receiveData()) {
      LOG_TRACE("Connection closed for client fd", clientFd);
      pollManager.removeFd(it->getFd());
      clients.erase(it);
    }
  }
}

// void Server::handleClient(PollManager& pollManager, int clientFd) {
//   Client* client = nullptr;
//   for (Client& c : clients) {
//     if (c.getFd() == clientFd) {
//       client = &c;
//       break;
//     }
//   }

//   if (client) {
//     if (!client->receiveData()) {
//       pollManager.removeFd(clientFd);
//       LOG_INFO("connection closed for client fd ", clientFd);
//       auto it = std::find(clients.begin(), clients.end(), *client);
//       if (it != clients.end()) {
//         clients.erase(it);
//       }
//     }
//   }
// }

void Server::cleanup(void) { close(sockFd); }