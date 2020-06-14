
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once
#include "noncopyable.h"
#include <string>
#include <vector>
#include "fixed-buffer.h"
#include <memory>
#include "cur-thread.h"
#include "mutexlock.h"
#include "countdown-latch.h"
#include "condition.h"
#include <assert.h>
#include <string.h>
#include "mutexlock-guard.h"
#include "log-file.h"
#include "log-util.h"
namespace server {
class AsyncLogging : public noncopyable {
 public:
  AsyncLogging(const std::string basename, int flushInterval = 2);
  ~AsyncLogging();
  void append(const char* logline, int len);
  void setBasename(const std::string basename);
  void start();
  void stop();
 private:
  void threadFunc();
 private:
  typedef FixedBuffer<kLargeBuffer> Buffer;
  typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
  typedef std::shared_ptr<Buffer> BufferPtr;
  const int flushInterval_;
  bool running_;
  std::string basename_;
  Thread thread_;
  MutexLock mutex_;
  Condition cond_;
  BufferPtr currentBuffer_;
  BufferPtr nextBuffer_;
  BufferVector buffers_;
  CountDownLatch latch_;
};

}

