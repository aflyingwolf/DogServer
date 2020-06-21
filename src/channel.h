
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once
#include <functional>
#include <sys/epoll.h>
#include <memory>
namespace server {
class EventLoop;
class WebSocketData;
class Channel {
 public:
  typedef std::function<void()> CallBack;

  Channel(EventLoop *loop);
  Channel(EventLoop *loop, int fd);
  ~Channel();

  int getFd();
  void setFd(int fd);

  void setReadHandler(CallBack &&readHandler) { 
    readHandler_ = readHandler; 
  }

  void setWriteHandler(CallBack &&writeHandler) {
    writeHandler_ = writeHandler;
  }

  void setErrorHandler(CallBack &&errorHandler) {
    errorHandler_ = errorHandler;
  }

  void setConnHandler(CallBack &&connHandler) { 
    connHandler_ = connHandler; 
  }

  void handleEvents() {
    events_ = 0;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
      events_ = 0;
      return;
    }
    if (revents_ & EPOLLERR) {
      if (errorHandler_) 
        errorHandler_();
      events_ = 0;
      return;
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
      handleRead();
    }
    if (revents_ & EPOLLOUT) {
      handleWrite();
    }
    handleConn();
  }

  void handleRead();
  void handleWrite();
  void handleConn();

  void setRevents(__uint32_t ev) { 
    revents_ = ev; 
  }

  void setEvents(__uint32_t ev) { 
    events_ = ev; 
  }

  __uint32_t &getEvents() { 
    return events_; 
  }

  bool EqualAndUpdateLastEvents() {
    bool ret = (lastEvents_ == events_);
    lastEvents_ = events_;
    return ret;
  }
  __uint32_t getLastEvents() { 
    return lastEvents_; 
  }

 private:
  EventLoop *loop_;
  int fd_;

  __uint32_t events_;
  __uint32_t revents_;
  __uint32_t lastEvents_;

  CallBack readHandler_;
  CallBack writeHandler_;
  CallBack errorHandler_;
  CallBack connHandler_;

 public:
  std::shared_ptr<WebSocketData> holder_;

};

typedef std::shared_ptr<Channel> SP_Channel;

}

