/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#include <getopt.h>
#include <string>
#include "event-loop.h"
#include "server.h"
#include "logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
//#include <mcheck.h>
static void signal_handler(int signum) {
  switch (signum) {
    case SIGINT:
      printf("signum=SIGINT\n");
      break;
    case SIGPIPE:
      printf("signum=SIGPIPE\n");
      break;
  }
}

static void setup_signal_handlers() {
  struct sigaction act;
  act.sa_handler = signal_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGINT, &act, NULL);
  sigaction(SIGPIPE, &act, NULL);
}

int main(int argc, char *argv[]) {
  int threadNum = 1;
  int port = 8880;
  std::string logPath = "./WebServer.log";
  // parse args
  int opt;
  const char *str = "t:l:p:";
  while ((opt = getopt(argc, argv, str)) != -1) {
    switch (opt) {
      case 't': {
        threadNum = atoi(optarg);
        break;
      }
      case 'l': {
        logPath = optarg;
        if (logPath.size() < 2 || optarg[0] != '/') {
          printf("logPath should start with \"/\"\n");
          abort();
        }
        break;
      }
      case 'p': {
        port = atoi(optarg);
        break;
      }
      default:
        break;
    }
  }
  //setup_signal_handlers();
  //setenv("MALLOC_TRACE", "mtrace_output.txt", 1);
  //mtrace();
  server::Logger::setLogFileName(logPath);
  server::EventLoop mainLoop;
  server::Server websocketServer(&mainLoop, threadNum, port);
  websocketServer.start();
  mainLoop.loop();
  return 0;
}
