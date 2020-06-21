
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#include "cur-epoll.h"
#include <unistd.h>
#include "logging.h"
#include "cur-thread.h"
namespace server {

Epoll::Epoll() : epollFd_(epoll_create1(EPOLL_CLOEXEC)), events_(EVENTSNUM) {
  assert(epollFd_ > 0); 
}

Epoll::~Epoll() {
  if (epollFd_ > 0) {
    close(epollFd_);
  }
}

void Epoll::epoll_add(SP_Channel request, int timeout) {
  int fd = request->getFd();
  struct epoll_event event;
  event.data.fd = fd; 
  event.events = request->getEvents();

  request->EqualAndUpdateLastEvents();

  fd2chan_[fd] = request;
  if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0) {
    LOG_E << "epoll_add error";
    fd2chan_[fd].reset();
  }
}

void Epoll::epoll_mod(SP_Channel request, int timeout) {
  int fd = request->getFd();
  if (!request->EqualAndUpdateLastEvents()) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();
    if (epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0) {
      LOG_E << "epoll_mod error";
      fd2chan_[fd].reset();
    }
  }
}

void Epoll::epoll_del(SP_Channel request) {
  int fd = request->getFd();
  struct epoll_event event;
  event.data.fd = fd;
  event.events = request->getLastEvents();
  if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0) {
    LOG_E << "epoll_del error";
  }
  fd2chan_[fd].reset();
}

std::vector<SP_Channel> Epoll::poll() {
  while (true) {
    int event_count =
        epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
    if (event_count < 0) {
      LOG_E << "epoll wait error:" << event_count << "tid:" << tid();
    }
    std::vector<SP_Channel> req_data = getEventsRequest(event_count);
    if (req_data.size() > 0) {
      return req_data;
    }
  }
}

void Epoll::handleExpired() {
  //timerManager_.handleExpiredEvent();
}

std::vector<SP_Channel> Epoll::getEventsRequest(int events_num) {
  std::vector<SP_Channel> req_data;
  for (int i = 0; i < events_num; ++i) {
    // 获取有事件产生的描述符
    int fd = events_[i].data.fd;

    SP_Channel cur_req = fd2chan_[fd];

    if (cur_req) {
      cur_req->setRevents(events_[i].events);
      cur_req->setEvents(0);
      req_data.push_back(cur_req);
    } else {
      LOG_E << "SP cur_req is invalid";
    }
  }
  return req_data;
}

}

