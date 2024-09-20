#include <PollManager.hpp>

PollManager::PollManager(void) {
  LOG_DEBUG(Utility::getConstructor(*this));
  epollEvents.resize(MAX_EVENTS);
  epollFd = epoll_create(MAX_CLIENTS);
  if (epollFd == -1) {
    throw serverError("Failed to create epollFd");
  }
}

PollManager::~PollManager(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
  close(epollFd);
}

void PollManager::addFd(int fd, uint32_t events) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;

  if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
    throw serverError("Failed to add fd to epollFd");
  }
  LOG_DEBUG("Added fd:", fd, "to epollFd");
}

void PollManager::removeFd(int fd) {
  if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    throw serverError("Failed to remove fd from epollFd");
  }
  LOG_DEBUG("Removed fd:", fd, "from epollFd");
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
  auto it = std::find_if(epollEvents.begin(), epollEvents.end(),
                         [fd](const struct epoll_event& event) { return event.data.fd == fd; });
  return it != epollEvents.end();
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
