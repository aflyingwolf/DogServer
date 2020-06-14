
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once
#include <time.h>
#include "noncopyable.h"
#include <stdio.h>
#include <string>
namespace server {

class AppendFile : public noncopyable {
 public:
  explicit AppendFile(std::string filename);
  ~AppendFile();
  //size_t是标准C库中定义的，在64位系统中为long long unsigned int，非64位系统中为long unsigned int
  void append(const char *logline, const size_t len);
  void flush();

 private:
  size_t write(const char *logline, size_t len);
  FILE *fp_;
  char buffer_[64 * 1024];
};

}

