
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once
namespace server {

enum ERROR_TYPE {
  ERROR_NO = 0,
  ERROR_CLIENT_CLOSE,
  ERROR_INTERNAL,
  ERROR_PARSE_DATA,
  ERROR_READ,
  ERROR_WRITE,
  ERROR_PARSE_HEAD = 400
};


enum ProcessState {
  STATE_HEAD = 1,
  STATE_START,
  STATE_DATA,
  STATE_FINISH,
  STATE_CLOSE,
  STATE_ERROR
};

enum WEBSOCKET_FIN_STATUS {
  WEBSOCKET_FIN_MSG_END      =1,  //该消息为消息尾部
  WEBSOCKET_FIN_MSG_NOT_END  =0,  //还有后续数据包
};

enum WEBSOCKET_OPCODE_TYPE {
  WEBSOCKET_APPEND_DATA      =0X0, //表示附加数据
  WEBSOCKET_TEXT_DATA        =0X1, //表示文本数据
  WEBSOCKET_BINARY_DATA      =0X2, //表示二进制数据
  WEBSOCKET_CONNECT_CLOSE    =0X8, //表示连接关闭
  WEBSOCKET_PING_PACK        =0X9, //表示ping
  WEBSOCKET_PANG_PACK        =0XA, //表示表示pong
};

enum WEBSOCKET_MASK_STATUS {
  WEBSOCKET_NEED_MASK        =1,  //需要掩码处理
  WEBSOCKET_NEED_NOT_MASK    =0,  //不需要掩码处理
};

const __uint32_t DEFAULT_EVENT = EPOLLIN;
const int DEFAULT_EXPIRED_TIME = 2000; //ms

#define MAGIC_KEY "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = -1;

const int MAXFDS = 100000;
}
