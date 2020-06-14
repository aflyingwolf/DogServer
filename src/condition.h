
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once
#include "mutexlock.h"
#include <pthread.h>
#include <errno.h>
#include "mutexlock-guard.h"

namespace server {

class Condition : public noncopyable {
 public:
  explicit Condition(MutexLock &_mutex) : mutex_(_mutex) {
    pthread_cond_init(&cond_, NULL);
  }
  ~Condition() { pthread_cond_destroy(&cond_); }
  void wait() { pthread_cond_wait(&cond_, mutex_.get()); }
  void notify() { pthread_cond_signal(&cond_); }
  void notifyAll() { pthread_cond_broadcast(&cond_); }
  bool waitForSeconds(int seconds) {
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += static_cast<time_t>(seconds);
    return ETIMEDOUT == pthread_cond_timedwait(&cond_, mutex_.get(), &abstime);
  }

 private:
  MutexLock &mutex_;
  pthread_cond_t cond_;
};

}

