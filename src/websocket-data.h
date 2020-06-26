
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once
#include <memory>
#include "event-loop.h"
#include "channel.h"
#include "packet-buffer.h"
#include <map>
#include "dog-status.h"
#include <unistd.h>
#include "util.h"
#include "logging.h"
#ifdef _ADD_ASR 
#include "Dog-Decoder.h"
#endif
namespace server {

class WebSocketData : public std::enable_shared_from_this<WebSocketData> {
 public:
  WebSocketData(EventLoop *loop, int connfd);
  ~WebSocketData();
  void handleClose();
  void newEvent();
  #ifdef _ADD_ASR
  int addDecoder(dog::DogResource *res);
  #endif
 private:
  int readAndParseHead();
  int sendRespond();
  int processStart();
  int readAndParsePacket();
  int processPacket();
  int processEnd();
  void processError(ERROR_TYPE error_type);
  void sendErrorRespond(int err_code);
  int writeData(std::string &data);
  int writeData(std::shared_ptr<Packet> packet);
  int parseHead(std::string &head);
  int sendResult(const char *result, int length);

 public:
  std::shared_ptr<Channel> getChannel() { return channel_; }
  EventLoop *getLoop() { return loop_; }

 private:
  EventLoop *loop_;
  std::shared_ptr<Channel> channel_;
  int fd_;
  std::shared_ptr<Packet> readPacket_;
  std::shared_ptr<Packet> writePacket_;
  ERROR_TYPE error_;
  ProcessState state_;
  bool is_final_;
  std::map<std::string, std::string> headers_;
  std::string uuid_;
  int this_conn_bytes_;
  #ifdef _ADD_ASR
  dog::DogDecoder *asrDecoder_;
  #endif
 private:
  void handleRead();
  void handleConn();
};

}

