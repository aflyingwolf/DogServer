
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#include "util.h"
#include <sys/socket.h>
#include <sys/types.h>
#include "logging.h"
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
namespace server {

int socket_bind_listen(int port) {
  // 检查port值，取正确区间范围
  if (port < 0 || port > 65535) {
    LOG_E << "not valid port:" << port;
    return -1;
  }

  // 创建socket(IPv4 + TCP)，返回监听描述符
  int listen_fd = 0;
  if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    LOG_E << "create socket failed.";
    return -1;
  }

  // 消除bind时"Address already in use"错误
  int optval = 1;
  if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optval,
                 sizeof(optval)) == -1) {
    LOG_E << "set reuse addr failed.";
    close(listen_fd);
    return -1; 
  }

  // 设置服务器IP和Port，和监听描述副绑定
  struct sockaddr_in server_addr;
  bzero((char *)&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons((unsigned short)port);
  if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    LOG_E << "bind socket failed.";
    close(listen_fd);
    return -1; 
  }
  // 开始监听，最大等待队列长为LISTENQ
  if (listen(listen_fd, 2048) == -1) {
    LOG_E << "listen socket failed.";
    close(listen_fd);
    return -1; 
  }

  // 无效监听描述符
  if (listen_fd == -1) {
    LOG_E << "valid socket fd.";
    close(listen_fd);
    return -1;
  }
  return listen_fd;
}

int setSocketNonBlocking(int fd) {
  int flag = fcntl(fd, F_GETFL, 0); 
  if (flag == -1) return -1; 

  flag |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flag) == -1) return -1; 
  return 0;
}

void setSocketNodelay(int fd) {
  int enable = 1;
  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&enable, sizeof(enable));
}

std::string gen_uuid() {
  std::string guid("");
  uuid_t uuid;
  char str[50] = {}; 
  uuid_generate(uuid);
  uuid_unparse(uuid, str);
  guid.assign(str);
  return guid;
}

}

