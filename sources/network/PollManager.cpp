#include "PollManager.hpp"

PollManager::PollManager(void) {
  LOG_DEBUG(Utility::getConstructor(*this));
  pollFds.reserve(MAX_CLIENTS);
}

PollManager::~PollManager(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
}

void PollManager::addFd(int fd, short events) {
  struct pollfd pFd = {fd, events, 0};
  pollFds.push_back(pFd);
  LOG_DEBUG("Added fd:", fd, "to pollFds");
}

void PollManager::removeFd(int fd) {
  auto it = std::find_if(pollFds.begin(), pollFds.end(),
                         [fd](const pollfd& pfd) { return pfd.fd == fd; });
  if (it != pollFds.end()) {
    pollFds.erase(it);
    LOG_DEBUG("Removed fd:", fd, "from pollFds");
  }
}

int PollManager::pollFdsCount(void) {
  return poll(pollFds.data(), pollFds.size(), TIMEOUT);
}

std::vector<struct pollfd>& PollManager::getPollFds() {
  return pollFds;
}