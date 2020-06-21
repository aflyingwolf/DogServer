
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once
#include "condition.h"
#include "event-loop.h"
#include "cur-thread.h"
namespace server {

class EventLoopThread : public noncopyable {
 public:
  EventLoopThread();
  ~EventLoopThread();
  EventLoop* startLoop();

 private:
  void threadFunc();

 private:
  EventLoop* loop_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
};

}

