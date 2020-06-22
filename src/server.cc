
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#include "server.h"
#include <arpa/inet.h>
namespace server {

Server::Server(EventLoop *loop, int threadNum, int port)
    : loop_(loop),
      threadNum_(threadNum),
      eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)),
      acceptChannel_(new Channel(loop_)),
      port_(port),
      listenFd_(socket_bind_listen(port_)) {
  acceptChannel_->setFd(listenFd_);
  if (setSocketNonBlocking(listenFd_) < 0) {
    LOG_E << "set socket non block failed";
    abort();
  }
}

void Server::start() {
  eventLoopThreadPool_->start();
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);
  acceptChannel_->setReadHandler(std::bind(&Server::handNewConn, this));
  acceptChannel_->setConnHandler(std::bind(&Server::handThisConn, this));
  loop_->addToPoller(acceptChannel_, 0); 
}

void Server::handNewConn() {
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(struct sockaddr_in));
  socklen_t client_addr_len = sizeof(client_addr);
  int accept_fd = 0;
  while ((accept_fd = accept(listenFd_, (struct sockaddr *)&client_addr,
                             &client_addr_len)) > 0) {
    EventLoop *loop = eventLoopThreadPool_->getNextLoop();
    LOG_I << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
        << ntohs(client_addr.sin_port);
    // 限制服务器的最大并发连接数
    if (accept_fd >= MAXFDS) {
      LOG_W << "accept fd too big:" << accept_fd;
      close(accept_fd);
      continue;
    }
    // 设为非阻塞模式
    if (setSocketNonBlocking(accept_fd) < 0) {
      LOG_E << "Set non block failed!";
      return;
    }

    setSocketNodelay(accept_fd);

    std::shared_ptr<WebSocketData> req_info(new WebSocketData(loop, accept_fd));
    std::shared_ptr<Channel> new_channel = req_info->getChannel();
    new_channel->holder_ = req_info;
    loop->queueInLoop(std::bind(&WebSocketData::newEvent, req_info));
  }
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}

void Server::handThisConn() {
  loop_->updatePoller(acceptChannel_);
}

}

