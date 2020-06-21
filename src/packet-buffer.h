
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once
namespace server {

enum PacketState {
  PACKET_INIT = 1,
  PACKET_HEAD_126,
  PACKET_HEAD_127,
  PACKET_MASK,
  PACKET_DATA
};

class Packet {
 public:
  Packet(){
    reset();
  }
  ~Packet(){
    if (data_) {
      free(data_);
      data_ = NULL;
    }   
  }
  void reset() {
    data_ = NULL;
    len_ = 0;
    size_ = 0;
    fin_ = 0;
    opcode_ = 0;
    mask_ = 0;
    body_size_ = 0;
    head_size_ = 0;
    state_ = PACKET_INIT;
  }
  void resize(int len) {
    if (len > size_) {
      data_ = (char*)realloc(data_, len);
      size_ = len;
    }   
  }
  char *buffer() {
    return data_;
  }
  char *data_;
  int len_;
  int size_;
  uint8_t fin_;
  uint8_t opcode_;
  uint8_t mask_;
  uint8_t masking_key_[4];
  uint64_t body_size_;
  int head_size_;
  PacketState state_; 
};

}

