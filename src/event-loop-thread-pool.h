
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once
#include "event-loop.h"
#include <vector>
#include "event-loop-thread.h"
#include "logging.h"
namespace server {

class EventLoopThreadPool : public noncopyable {
 public:
  EventLoopThreadPool(EventLoop* baseLoop, int numThreads);
  ~EventLoopThreadPool() { LOG_D << "~EventLoopThreadPool()"; }
  void start();
  EventLoop* getNextLoop();
 private:
  EventLoop* baseLoop_;
  bool started_;
  int numThreads_;
  int next_;
  std::vector<std::shared_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;
};

}

