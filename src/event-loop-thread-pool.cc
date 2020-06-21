
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#include "event-loop-thread-pool.h"
namespace server {

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, int numThreads)
    : baseLoop_(baseLoop), started_(false), numThreads_(numThreads), next_(0) {
  if (numThreads_ <= 0) {
    LOG_E << "numThreads_ <= 0";
    abort();
  }
}

void EventLoopThreadPool::start() {
  baseLoop_->assertInLoopThread();
  for (int i = 0; i < numThreads_; ++i) {
    std::shared_ptr<EventLoopThread> t(new EventLoopThread());
    threads_.push_back(t);
    loops_.push_back(t->startLoop());
  }
  started_ = true;
}

EventLoop *EventLoopThreadPool::getNextLoop() {
  baseLoop_->assertInLoopThread();
  assert(started_);
  EventLoop *loop = baseLoop_;
  if (!loops_.empty()) {
    loop = loops_[next_];
    next_ = (next_ + 1) % numThreads_;
  }
  return loop;
}

}

