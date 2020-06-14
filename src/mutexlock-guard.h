
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once
namespace server {

class MutexLockGuard : public noncopyable {
 public:
  //防止类构造函数的隐式自动转换.
  explicit MutexLockGuard(MutexLock &_mutex) : mutex_(_mutex) { mutex_.lock(); }
  ~MutexLockGuard() { mutex_.unlock(); }
 private:
  MutexLock &mutex_;
};

}

