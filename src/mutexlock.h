
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once

#include "noncopyable.h"
#include <pthread.h>
#include <string.h>
namespace server {

class MutexLock : private noncopyable {
 public:
  MutexLock() { pthread_mutex_init(&mutex_, NULL); }
  ~MutexLock() {
    pthread_mutex_lock(&mutex_);
    pthread_mutex_destroy(&mutex_);
  }
  void lock() { pthread_mutex_lock(&mutex_); }
  void unlock() { pthread_mutex_unlock(&mutex_); }
  pthread_mutex_t *get() { return &mutex_; }
 private:
  pthread_mutex_t mutex_;
  //友元类不受访问权限影响
  friend class Condition;
};

}

