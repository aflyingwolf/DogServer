
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#include "async-logging.h"
#include <time.h>
#include <unistd.h>
namespace server {

AsyncLogging::AsyncLogging(std::string logFileName_, int flushInterval)
    : flushInterval_(flushInterval),
      running_(false),
      basename_(logFileName_),
      thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
      mutex_(),
      cond_(mutex_),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer),
      buffers_(),
      latch_(1) {
  assert(logFileName_.size() > 1); 
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

AsyncLogging::~AsyncLogging() {
  if (running_) {
    stop();
  }   
}

void AsyncLogging::setBasename(const std::string basename) {
  basename_ = basename;
}

void AsyncLogging::stop() {
  running_ = false;
  cond_.notify();
  thread_.join();
}

void AsyncLogging::start() {
  running_ = true;
  thread_.start();
  latch_.wait();
}

void AsyncLogging::append(const char* logline, int len) {
  MutexLockGuard lock(mutex_);
  if (currentBuffer_->avail() > len)
    currentBuffer_->append(logline, len);
  else {
    buffers_.push_back(currentBuffer_);
    currentBuffer_.reset();
    if (nextBuffer_)
      currentBuffer_ = std::move(nextBuffer_);
    else
      currentBuffer_.reset(new Buffer);
    currentBuffer_->append(logline, len);
    cond_.notify();
  }
}

void AsyncLogging::threadFunc() {
  assert(running_ == true);
  LogFile output(basename_);
  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  newBuffer1->bzero();
  newBuffer2->bzero();
  BufferVector buffersToWrite;
  buffersToWrite.reserve(16);
  latch_.countDown();
  while (running_) {
    assert(newBuffer1 && newBuffer1->length() == 0);
    assert(newBuffer2 && newBuffer2->length() == 0);
    assert(buffersToWrite.empty());

    {
      MutexLockGuard lock(mutex_);
      if (buffers_.empty())  // unusual usage!
      {
        cond_.waitForSeconds(flushInterval_ * 10000);
      }
      buffers_.push_back(currentBuffer_);
      currentBuffer_.reset();

      currentBuffer_ = std::move(newBuffer1);
      buffersToWrite.swap(buffers_);
      if (!nextBuffer_) {
        nextBuffer_ = std::move(newBuffer2);
      }
    }

    assert(!buffersToWrite.empty());

    if (buffersToWrite.size() > 25) {
      char buf[256];
      snprintf(buf, sizeof buf, "Dropped log messages, %zd larger buffers\n",
        /*Timestamp::now().toFormattedString().c_str(), */buffersToWrite.size()-2);
      fputs(buf, stderr);
      output.append(buf, static_cast<int>(strlen(buf)));
      buffersToWrite.erase(buffersToWrite.begin() + 2, buffersToWrite.end());
    }

    for (size_t i = 0; i < buffersToWrite.size(); ++i) {
      // FIXME: use unbuffered stdio FILE ? or use ::writev ?
      output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
    }
    if (buffersToWrite.size() > 2) {
      // drop non-bzero-ed buffers, avoid trashing
      buffersToWrite.resize(2);
    }

    if (!newBuffer1) {
      assert(!buffersToWrite.empty());
      newBuffer1 = buffersToWrite.back();
      buffersToWrite.pop_back();
      newBuffer1->reset();
    }

    if (!newBuffer2) {
      assert(!buffersToWrite.empty());
      newBuffer2 = buffersToWrite.back();
      buffersToWrite.pop_back();
      newBuffer2->reset();
    }

    buffersToWrite.clear();
    output.flush();
  }
  output.flush();
}

}

