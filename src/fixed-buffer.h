
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once
#include <string.h>
#include "noncopyable.h"
#include <stdio.h>
namespace server {

const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

template <int SIZE>
class FixedBuffer : public noncopyable {
 public:
  FixedBuffer() : cur_(data_) {}
  ~FixedBuffer() {}
  //由外面控制buffer可用
  void append(const char* buf, size_t len) {
    if (avail() > static_cast<int>(len)) {
      memcpy(cur_, buf, len);
      cur_ += len;
    } else {
      fprintf(stderr, "fixed buffer is small!\n");
    }
  }
  const char* data() const { 
    return data_;
  }
  int length() const { 
    return static_cast<int>(cur_ - data_); 
  }
  char* current() { 
    return cur_; 
  }
  int avail() const { 
    return static_cast<int>(end() - cur_); 
  }
  void add(size_t len) { 
    cur_ += len; 
  }
  void reset() { 
    cur_ = data_; 
  }
  void bzero() { 
    memset(data_, 0, sizeof(data_)); 
  }
 private:
  //在一个类的函数后面加上const后，就表明这个函数是不能改变类的成员变量的（加了mutable修饰的除外）。
  //实际上，也就是对这个this指针加上了const修饰
  const char* end() const { 
    return data_ + sizeof(data_); 
  }
 private:
  char data_[SIZE];
  char* cur_;
};

}

