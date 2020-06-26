
/*
@author Jiang W
@time   2020.6.13
@email  aflyingwolf@126.com
**/

#include "logging.h"
#include <unistd.h>
using namespace server;

int main(int argc, char *argv) {
  Logger::setLogFileName("webserver.log");
  for (int i = 0; i < 500000; i++) {
    LOG_I << i;
  }
  for (short i = 0; i < 5000; i++) {
    LOG_E << i;
  }
  for (short i = 0; i < 5000; i++) {
    LOG_W << (i % 2 == 1 ? true : false);
  }
  for (int i = 0; i < 300; i++) {
    LOG_D << "a flying wolf!";
  }
  for (int i = 0; i < 300; i++) {
    LOG_I << 3.1415926;
  }
  for (int i = 0; i < 3; i++) {
    LOG_D << 'b';
  }
  return 0;
}

