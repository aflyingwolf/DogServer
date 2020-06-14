
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#pragma once
#include "log-stream.h"
namespace server {

class Logger {
 public:
  Logger(const char *fileName, int line);
  ~Logger();
  LogStream &stream() { return impl_.stream_; }
  static void setLogFileName(std::string fileName) { 
    logFileName_ = fileName; 
  }
  static std::string getLogFileName() { 
    return logFileName_; 
  }
 private:
  class Impl {
   public:
    Impl(const char *fileName, int line);
    void formatTime();
    LogStream stream_;
    int line_;
    std::string basename_;
  };
  Impl impl_;
  static std::string logFileName_;
};

#define LOG_E Logger(__FILE__, __LINE__).stream() << "Error: "
#define LOG_I Logger(__FILE__, __LINE__).stream() << "Info : "
#define LOG_D Logger(__FILE__, __LINE__).stream() << "Debug: "
#define LOG_W Logger(__FILE__, __LINE__).stream() << "Warn : "

}

