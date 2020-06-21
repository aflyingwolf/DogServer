
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#include "event-loop-thread.h"
namespace server {

EventLoopThread::EventLoopThread()
    : loop_(NULL),
      thread_(std::bind(&EventLoopThread::threadFunc, this), "EventLoopThread"),
      mutex_(),
      cond_(mutex_) {}

EventLoopThread::~EventLoopThread() {
  if (loop_ != NULL) {
    loop_->quit();
    thread_.join();
  }
}

EventLoop* EventLoopThread::startLoop() {
  assert(!thread_.started());
  thread_.start();
  {
    MutexLockGuard lock(mutex_);
    // 一直等到threadFun在Thread里真正跑起来
    while (loop_ == NULL) { 
      cond_.wait();
    }
  }
  return loop_;
}

void EventLoopThread::threadFunc() {
  EventLoop loop;

  {
    MutexLockGuard lock(mutex_);
    loop_ = &loop;
    cond_.notify();
  }

  loop.loop();
  loop_ = NULL;
}

}

