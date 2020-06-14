
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once
#include "condition.h"

namespace server {

class CountDownLatch : public noncopyable {
 public:
  explicit CountDownLatch(int count);
  void wait();
  void countDown();

 private:
  mutable MutexLock mutex_;
  Condition condition_;
  int count_;
};

}

