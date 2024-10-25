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
  /*   for (auto& fd : interestFdsList) {
    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
      continue;
    }
    LOG_DEBUG("Removed fd:", fd, "from epollFd");
  } */
  close(epollFd);
}

void PollManager::addFd(int fd, uint32_t events, std::function<void(int)> cleanUp) {
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
  LOG_DEBUG("Added fd:", fd, "to epollFd");
  interestFdsList[fd] = cleanUp;
}

void PollManager::removeFd(int fd) {

  auto it = interestFdsList.find(fd);
  if (it != interestFdsList.end() && (fcntl(fd, F_GETFD) != -1)) {

    if (epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr) == -1) {
      throw serverError("Failed to remove fd from epollFd");
    }
    LOG_DEBUG("Removed fd:", fd, "from epollFd");
    it->second(fd);
    interestFdsList.erase(it);
  }

  for (auto it = epollEvents.begin(); it != epollEvents.end(); it++) {
    if (it->data.fd == fd) {
      it->data.fd = -1;
    }
  }

  for (auto it = g_CgiParams.begin(); it != g_CgiParams.end(); it++) {
    if (it->outReadFd == fd || it->inWriteFd == fd) {
      g_CgiParams.erase(it);
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
  if (fd < 0) {
    return true;
  }

  return interestFdsList.find(fd) != interestFdsList.end();
}

int PollManager::epollWait(void) {
  int numEvents;
  while (!Utility::signalReceived()) {
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
