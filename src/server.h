
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once
#include "websocket-data.h"
#include "event-loop.h"
#include "event-loop-thread-pool.h"
#ifdef _ADD_ASR 
#include "Dog-Decoder.h"
#endif
namespace server {

class Server {
 public:
  Server(EventLoop *loop, int threadNum, int port);
  ~Server();
  EventLoop *getLoop() const {
    return loop_;
  }
  void start(std::string conf = "conf/dog.conf");
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
  #ifdef _ADD_ASR
  dog::DogResource *asrRes_;
  #endif
};
}

