
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once

#include "channel.h"
#include "cur-epoll.h"
#include <functional>
#include <memory>
#include "cur-thread.h"
namespace server {

class EventLoop {
 public:
  typedef std::function<void()> Functor;
  EventLoop();
  ~EventLoop();
  void loop();
  void quit();
  void runInLoop(Functor&& cb);
  void queueInLoop(Functor&& cb);
  bool isInLoopThread() const { return threadId_ == tid(); }
  void assertInLoopThread() { assert(isInLoopThread()); }
  void removeFromPoller(SP_Channel channel) {
    poller_->epoll_del(channel);
  }
  void updatePoller(SP_Channel channel, int timeout = 0) {
    poller_->epoll_mod(channel, timeout);
  }
  void addToPoller(SP_Channel channel, int timeout = 0) {
    poller_->epoll_add(channel, timeout);
  }
 private:
  std::shared_ptr<Epoll> poller_;
  int wakeupFd_;
  bool quit_;
  bool eventHandling_;
  mutable MutexLock mutex_;
  std::vector<Functor> pendingFunctors_;
  bool callingPendingFunctors_;
  const pid_t threadId_;
  SP_Channel pwakeupChannel_;
 private:
  void wakeup();
  void handleRead();
  void handleConn();
  void doPendingFunctors();
 private:
  int debug_time_;
};

}

