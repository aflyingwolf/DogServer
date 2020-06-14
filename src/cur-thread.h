
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once
#include "countdown-latch.h"
#include <functional>

namespace server {

extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;
void cacheTid();

inline int tid() {
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

inline const char* tidString() {
  return t_tidString;
}

inline int tidStringLength() {
  return t_tidStringLength;
}

inline const char* name() { 
  return t_threadName; 
}


class Thread : public noncopyable {
 public:
  typedef std::function<void()> ThreadFunc;
  explicit Thread(const ThreadFunc&, const std::string& name = std::string());
  ~Thread();
  void start();
  int join();
  bool started() const { return started_; }
  pid_t tid() const { return tid_; }
  const std::string& name() const { return name_; }
 private:
  void setDefaultName();
 private:
  bool started_;
  bool joined_;
  pthread_t pthreadId_;
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  CountDownLatch latch_;
};

}

