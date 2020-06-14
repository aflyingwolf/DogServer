
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once
#include "log-util.h"
#include "noncopyable.h"
#include <memory>
#include "mutexlock.h"
namespace server {

class LogFile : public noncopyable {

 public:
  LogFile(const std::string& basename, int flushEveryN = 1024);
  ~LogFile();
  void append(const char* logline, int len);
  void flush();

 private:
  void append_unlocked(const char* logline, int len);

 private:
  //const 告诉编译器某值是保持不变的
  const std::string basename_;
  const int flushEveryN_;
  int count_;

  //任意时刻unique_ptr只能指向某一个对象，指针销毁时，指向的对象也会被删除
  //禁止拷贝和赋值（底层实现拷贝构造函数和复制构造函数 = delete）
  //可以使用std::move()、unique_ptr.reset(...) 转移对象指针控制权
  std::unique_ptr<MutexLock> mutex_;
  std::unique_ptr<AppendFile> file_;
  
};

}

