#include <Global.hpp>
#include <PollManager.hpp>

PollManager::PollManager(void) {
  LOG_DEBUG(Utility::getConstructor(*this));
  interestFdsList.clear();
  epollEvents.resize(MAX_EVENTS);
  epollFd = epoll_create(MAX_CLIENTS);
  if (epollFd == -1) {
    throw serverError("Failed to create epollFd");
  }
  Utility::setCloseOnExec(epollFd);
}

PollManager::~PollManager(void) {
  LOG_DEBUG(Utility::getDeconstructor(*this));
  for (auto it = interestFdsList.begin(); it != interestFdsList.end(); it++) {
    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, it->first, nullptr) == -1) {
      LOG_DEBUG("Failed to remove fd from epollFd");
    }
    LOG_DEBUG("Removed fd:", it->first, "from epollFd");
    it->second(it->first);
  }
  LOG_DEBUG("Closed epollFd");
  close(epollFd);
}

void PollManager::addFd(int fd, uint32_t events,
                        std::function<void(int)> cleanUp) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  if (fd < 0) {
    return;
  }
  if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event) == -1) {
    if (errno == EEXIST) {
      LOG_DEBUG("Fd:", fd, "already exists in epollFd");
      return;
    } else {
      throw serverError("Failed to add fd to epollFd");
    }
  }
  interestFdsList[fd] = cleanUp;
  LOG_DEBUG("Added fd:", fd, "to epollFd");
}

void PollManager::removeFd(int fd) {
  auto it = interestFdsList.find(fd);
  if (it != interestFdsList.end() && (fcntl(fd, F_GETFD) != -1)) {
    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
      LOG_ERROR("Failed to remove fd from epollFd");
      return;
      // throw serverError("Failed to remove fd",fd," from epollFd");
    }
    LOG_DEBUG("Removed fd:", fd, "from epollFd");
    it->second(fd);
    interestFdsList.erase(it);
    for (auto it = epollEvents.begin(); it != epollEvents.end(); ++it) {
      if (it->data.fd == fd) {
        LOG_TRACE("Erasing fd:", fd, "from epollEvents");
        it->data.fd = -1;
      }
    }
    for (auto it = g_CgiParams.begin(); it != g_CgiParams.end();) {
      if (it->outReadFd == fd || it->inWriteFd == fd || it->outWriteFd == fd ||
          it->inReadFd == fd || it->clientFd == fd) {
        it = g_CgiParams.erase(it);  // Erase safely
      } else {
        ++it;  // Increment iterator only if not erased
      }
    }
  }
}

void PollManager::modifyFd(int fd, uint32_t events) {
  struct epoll_event event;
  event.data.fd = fd;
  event.events = events;
  if (epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &event) == -1) {
    LOG_ERROR("Failed to modify fd", fd, "in epollFd", "errno:", errno, ":",
              strerror(errno));
    return;
    // throw serverError("Failed to modify fd",fd, "in epollFd", "errno:",
    // errno, ":", strerror(errno));
  }
  LOG_DEBUG("Modified events for fd:", fd, "in epollFd");
}

bool PollManager::fdExists(int fd) {
  if (fd < 0) {
    return true;
  }
  return interestFdsList.find(fd) != interestFdsList.end();
}

int PollManager::epollWait(void) {
  int numEvents;
  while (!Utility::signalReceived()) {
    epollEvents.clear();
    epollEvents.resize(MAX_EVENTS);
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

std::map<int, std::function<void(int)>>& PollManager::getInterestFdsList(void) {
  return interestFdsList;
}

bool PollManager::isValidFd(int fd) {
  if (interestFdsList.find(fd) != interestFdsList.end() &&
      fcntl(fd, F_GETFD) != -1) {
    return true;
  } else {
    return false;
  }
}