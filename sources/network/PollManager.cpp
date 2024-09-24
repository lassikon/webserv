#include <PollManager.hpp>

PollManager::PollManager(void) {
  LOG_DEBUG(Utility::getConstructor(*this));
  interestFdsList.resize(MAX_CLIENTS);
  interestFdsList.clear();
  epollEvents.resize(MAX_EVENTS);
  epollFd = epoll_create(MAX_CLIENTS);
  if (epollFd == -1) {
    throw serverError("Failed to create epollFd");
  }
}

PollManager::~PollManager(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
  for (auto& fd : interestFdsList) {
    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
      continue;
    }
    LOG_DEBUG("Removed fd:", fd, "from epollFd");
  }
  close(epollFd);
}

void PollManager::addFd(int fd, uint32_t events) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;

  if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
    if (errno == EEXIST) {
      LOG_DEBUG("Fd:", fd, "already exists in epollFd");
      return;
    } else {
      throw serverError("Failed to add fd to epollFd");
    }
  }
  LOG_DEBUG("Added fd:", fd, "to epollFd");
  interestFdsList.push_back(fd);
}

void PollManager::removeFd(int fd) {
  for (auto it = epollEvents.begin(); it != epollEvents.end(); ++it) {
    if (it->data.fd == fd) {
      epollEvents.erase(it);
    }
  }

  if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    throw serverError("Failed to remove fd from epollFd");
  }
  LOG_DEBUG("Removed fd:", fd, "from epollFd");

  for (auto it = interestFdsList.begin(); it != interestFdsList.end(); ++it) {
    if (*it == fd) {
      interestFdsList.erase(it);
      break;
    }
  }
}

void PollManager::modifyFd(int fd, uint32_t events) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;

  if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event) == -1) {
    throw serverError("Failed to modify fd in epollFd");
  }
  LOG_DEBUG("Modified events for fd:", fd, "in epollFd");
}

bool PollManager::fdExists(int fd) {
  auto it = std::find(interestFdsList.begin(), interestFdsList.end(), fd);
  return it != interestFdsList.end();
}

int PollManager::epollWait(void) {
  int numEvents;
  while (true) {
    numEvents = epoll_wait(epollFd, epollEvents.data(), MAX_EVENTS, TIMEOUT);
    if (numEvents == -1) {
      if (errno == EINTR) {  // if interrupted by signal, try again
        continue;
      } else {
        throw serverError("Failed to poll fds");
      }
    }
    break;
  }
  return numEvents;
}

std::vector<struct epoll_event>& PollManager::getEpollEvents(void) {
  return epollEvents;
}

std::vector<int>& PollManager::getInterestFdsList(void) {
  return interestFdsList;
}
