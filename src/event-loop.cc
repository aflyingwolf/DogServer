
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#include "event-loop.h"
#include <sys/eventfd.h>
#include "logging.h"
#include <functional>
#include <unistd.h>
namespace server {

__thread EventLoop* t_loopInThisThread = 0;

int createEventfd() {
  int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (evtfd < 0) {
    LOG_E << "Failed in eventfd";
    abort();
  }
  return evtfd;
}

ssize_t readn(int fd, void *buff, size_t n) {
  size_t nleft = n;
  ssize_t nread = 0;
  ssize_t readSum = 0;
  char *ptr = (char *)buff;
  while (nleft > 0) {
    if ((nread = read(fd, ptr, nleft)) < 0) {
      if (errno == EINTR)
        nread = 0;
      else if (errno == EAGAIN) {
        return readSum;
      } else {
        return -1; 
      }   
    } else if (nread == 0)
      break;
    readSum += nread;
    nleft -= nread;
    ptr += nread;
  }
  return readSum;
}

ssize_t writen(int fd, void *buff, size_t n) {
  size_t nleft = n;
  ssize_t nwritten = 0;
  ssize_t writeSum = 0;
  char *ptr = (char *)buff;
  while (nleft > 0) {
    if ((nwritten = write(fd, ptr, nleft)) <= 0) {
      if (nwritten < 0) {
        if (errno == EINTR) {
          nwritten = 0;
          continue;
        } else if (errno == EAGAIN) {
          return writeSum;
        } else
          return -1;
      }
    }
    writeSum += nwritten;
    nleft -= nwritten;
    ptr += nwritten;
  }
  return writeSum;
}
EventLoop::EventLoop()
    : poller_(new Epoll()),
      wakeupFd_(createEventfd()),
      quit_(false),
      eventHandling_(false),
      idle(true),
      callingPendingFunctors_(false),
      threadId_(tid()),
      pwakeupChannel_(new Channel(this, wakeupFd_)) {
  if (t_loopInThisThread) {
    LOG_E << "Another EventLoop exists in this thread ";
  } else {
    t_loopInThisThread = this;
  }
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
  pwakeupChannel_->setReadHandler(std::bind(&EventLoop::handleRead, this));
  pwakeupChannel_->setConnHandler(std::bind(&EventLoop::handleConn, this));
  addToPoller(pwakeupChannel_, 0);
}

EventLoop::~EventLoop() {
  if (wakeupFd_ > 0) {
    close(wakeupFd_);
  }
  LOG_D << "~EventLoop()";
  removeFromPoller(pwakeupChannel_);
  t_loopInThisThread = NULL;
}

void EventLoop::loop() {
  assert(isInLoopThread());
  quit_ = false;
  std::vector<SP_Channel> ret;
  while (!quit_) {
    ret.clear();
    ret = poller_->poll();
    eventHandling_ = true;
    for (auto& it : ret)
      it->handleEvents();
    eventHandling_ = false;
    doPendingFunctors();
    poller_->handleExpired();
  }
}

void EventLoop::quit() {
  quit_ = true;
  if (!isInLoopThread()) {
    wakeup();
  }
}

void EventLoop::runInLoop(Functor&& cb) {
  if (isInLoopThread()) {
    cb();
  } else {
    queueInLoop(std::move(cb));
  }
}

void EventLoop::queueInLoop(Functor&& cb) {
  { 
    MutexLockGuard lock(mutex_);
    pendingFunctors_.emplace_back(std::move(cb));
  }
  if (!isInLoopThread() || callingPendingFunctors_) {
    wakeup();
  }
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof(one));
  if (n != sizeof(one)) {
    LOG_E << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handleRead() {
  uint64_t one = 1;
  ssize_t n = readn(wakeupFd_, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_E << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
  pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

void EventLoop::doPendingFunctors() {
  std::vector<Functor> functors;
  callingPendingFunctors_ = true;

  {
    MutexLockGuard lock(mutex_);
    functors.swap(pendingFunctors_);
  }

  for (size_t i = 0; i < functors.size(); ++i) {
    functors[i]();
  }
  callingPendingFunctors_ = false;
}

void EventLoop::handleConn() {
  updatePoller(pwakeupChannel_, 0);
}

}

