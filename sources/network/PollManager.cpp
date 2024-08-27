#include "PollManager.hpp"

PollManager::PollManager(void) {
  LOG_DEBUG("PollManager constructor called");
  pollFds.reserve(MAX_CLIENTS);
}

PollManager::~PollManager(void) { LOG_DEBUG("PollManager destructor called"); }

void PollManager::addFd(int fd, short events) {
  struct pollfd pFd = {fd, events, 0};
  pollFds.push_back(pFd);
  LOG_DEBUG("Added fd:", fd, "to pollFds");
}

void PollManager::removeFd(int fd) {
  pollFds.erase(
      std::remove_if(pollFds.begin(), pollFds.end(), [fd](pollfd& pfd) { return pfd.fd == fd; }),
      pollFds.end());
  LOG_DEBUG("Removed fd:", fd, "from pollFds");
}

int PollManager::pollFdsCount(void) { return poll(pollFds.data(), pollFds.size(), TIMEOUT); }

std::vector<struct pollfd>& PollManager::getPollFds() { return pollFds; }