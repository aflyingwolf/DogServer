
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#include "logging.h"
#include <string.h>
#include <time.h>
#include "async-logging.h"
#include <sys/time.h>
namespace server {

static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
//static AsyncLogging *AsyncLogger_;
static AsyncLogging AsyncLogger_("access.log");
std::string Logger::logFileName_ = "./access.log";

void once_init() {
  //AsyncLogger_ = new AsyncLogging(Logger::getLogFileName());
  //AsyncLogger_->start();
  AsyncLogger_.setBasename(Logger::getLogFileName());
  AsyncLogger_.start();
}

void output(const char* msg, int len) {
  pthread_once(&once_control_, once_init);
  //AsyncLogger_->append(msg, len);
  AsyncLogger_.append(msg, len);
}

Logger::Logger(const char *fileName, int line)
    : impl_(fileName, line) { }

Logger::~Logger() {
  impl_.stream_ << " -- " << impl_.basename_ << ':' << impl_.line_ << '\n';
  const LogStream::Buffer& buf(stream().buffer());
  output(buf.data(), buf.length());
}

Logger::Impl::Impl(const char *fileName, int line)
    : stream_(), line_(line), basename_(fileName) {
  formatTime();
}

void Logger::Impl::formatTime() {
  struct timeval  tv;
  struct timezone tz;
  struct tm *p;
  #define SIZE_OF_DATETIME 27
  char sys_time[SIZE_OF_DATETIME+1] = "";

  gettimeofday(&tv, &tz);
  p = localtime(&tv.tv_sec);
  sprintf(sys_time, "%04d-%02d-%02d %02d:%02d:%02d:%06ld ", 1900+p->tm_year, 1+p->tm_mon, p->tm_mday, p->tm_hour, p->tm_min, p->tm_sec, tv.tv_usec);
  //printf("strlen(sys_time)=[%lu]\n", strlen(sys_time));
  for(int i = strlen(sys_time); i < SIZE_OF_DATETIME; i++){
    sys_time[i] = '0';
  }
  sys_time[SIZE_OF_DATETIME] = '\0';
  stream_ << sys_time;

  /*
  struct timeval tv;
  time_t time;
  char str_t[26] = {0};
  gettimeofday (&tv, NULL);
  time = tv.tv_sec;
  struct tm* p_time = localtime(&time);
  strftime(str_t, 26, "%Y-%m-%d %H:%M:%S\n", p_time);
  stream_ << str_t;*/
}

}

