
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#include "log-util.h"

namespace server {

AppendFile::AppendFile(std::string filename) : fp_(fopen(filename.c_str(), "at")) {
  //在打开文件流后，读取内容之前，调用setbuffer（）可用来设置文件流的缓冲区
  setbuffer(fp_, buffer_, sizeof(buffer_));
}

AppendFile::~AppendFile() { fclose(fp_); }

void AppendFile::append(const char* logline, const size_t len) {
  //有一些时候，你不得不显示的加上this（当然这里不需要）因为函数参数名如果和成员变量重名，
  //编译器会优先选层级近的，这时候如果要用成员变量，需要显式的加上 this->
  size_t n = this->write(logline, len);
  size_t remain = len - n;
  while (remain > 0) {
    size_t x = write(logline + n, remain);
    if (x == 0) {
      int err = ferror(fp_);
      if (err) fprintf(stderr, "AppendFile::append() failed !\n");
      break;
    }
    n += x;
    remain = len - n;
  } 
}

void AppendFile::flush() { fflush(fp_); }

size_t AppendFile::write(const char* logline, size_t len) {
  //fwrite 和 fwrite_unlocked是一对，其中fwrite_unlocked是fwrite的线程不安全版本，因为不加锁。
  return fwrite_unlocked(logline, 1, len, fp_);
}

}

