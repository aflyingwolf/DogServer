
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once
#include "channel.h"
#include <memory>
#include <vector>
#include "dog-status.h"
#include <assert.h>
namespace server {

class Epoll {
 public:
  Epoll();
  ~Epoll();
  void epoll_add(SP_Channel request, int timeout);
  void epoll_mod(SP_Channel request, int timeout);
  void epoll_del(SP_Channel request);
  std::vector<SP_Channel> poll();
  std::vector<SP_Channel> getEventsRequest(int events_num);
  void handleExpired();
 private:
  int epollFd_;
  std::vector<epoll_event> events_;
  std::shared_ptr<Channel> fd2chan_[MAXFDS];
};

}

