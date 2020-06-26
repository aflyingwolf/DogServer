
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once
#include "websocket-data.h"
#include "event-loop.h"
#include "event-loop-thread-pool.h"
namespace server {

class Server {
 public:
  Server(EventLoop *loop, int threadNum, int port);
  ~Server() {}
  EventLoop *getLoop() const {
    return loop_;
  }
  void start();
  void handNewConn();
  void handThisConn();
 private:
  EventLoop *loop_;
  int threadNum_;
  std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;
  bool started_;
  std::shared_ptr<Channel> acceptChannel_;
  int port_;
  int listenFd_;
  int debug_num_;
};
}

