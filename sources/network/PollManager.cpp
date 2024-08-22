#include "PollManager.hpp"

PollManager::PollManager(void) { pollFDs.reserve(MAX_CLIENTS); }

PollManager::~PollManager(void) {}

void PollManager::addFD(int fd, short events) {
  struct pollfd pFD = {fd, events, 0};
  pollFDs.push_back(pFD);
}

void PollManager::removeFD(int fd) {
  pollFDs.erase(
      std::remove_if(pollFDs.begin(), pollFDs.end(), [fd](pollfd& pfd) { return pfd.fd == fd; }),
      pollFDs.end());
  LOG_INFO("connection closed for client fd ", fd);
}

int PollManager::pollFDsCount(void) { return poll(pollFDs.data(), pollFDs.size(), -1); }

std::vector<struct pollfd>& PollManager::getPollFDs() { return pollFDs; }
