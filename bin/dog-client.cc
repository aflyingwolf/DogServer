#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <sys/resource.h>
#include <pthread.h>
#include <vector>
#include <string.h>
#include "websocket-client.h"

using namespace client;
typedef struct _FileItem {
  char id[256];
  char path[1024];
} FileItem;

typedef struct _FileList {
  FileItem *list;
  int cur_pos;
  int list_num;
} FileList;

typedef struct _Param {
  char ip[16];
  int port;
  FileList *list;
  int epfd;
  int *thread_ids;
  int thread_id;
} Param;

using namespace std;

int CreateThread(void *(*start_routine)(void *), void *arg = NULL, pthread_t *thread = NULL, pthread_attr_t *pAttr = NULL) {
  pthread_attr_t thr_attr;
  if (pAttr == NULL) {
    pAttr = &thr_attr;
    pthread_attr_init(pAttr);
    pthread_attr_setstacksize(pAttr, 1024 * 1024); // 1 M的堆栈
    pthread_attr_setdetachstate(pAttr, PTHREAD_CREATE_DETACHED);
  }
  pthread_t tid;
  if (thread == NULL) {
    thread = &tid;
  }
  int r = pthread_create(thread, pAttr, start_routine, arg);
  pthread_attr_destroy(pAttr);
  return r;
}

static int SetRLimit() {
  struct rlimit rlim;
  rlim.rlim_cur = 20480;
  rlim.rlim_max = 20480;
  if (setrlimit(RLIMIT_NOFILE, &rlim) != 0) {
    perror("setrlimit");
  } else {
    ;//printf("setrlimit ok\n");
  }
  return 0;
}

int Setnonblocking(int sock) {
  int opts;
  opts = fcntl(sock,F_GETFL);
  if (opts<0) {
    return -1;
  }
  opts = opts|O_NONBLOCK;
  if (fcntl(sock,F_SETFL,opts)<0) {
    return -1;
  } 
  return 0;
}

pthread_mutex_t mute;
pthread_cond_t cond;

typedef struct _UserData {
  int fd;
  FileItem *item;
  int thread_id;
} UserData;

void *SendThread(void *arg) {

  Param *param = (Param*)arg;
  FileList *list = param->list;
  int thread_id = param->thread_id;
  while (1) {
    FileItem *item = NULL;
    pthread_mutex_lock(&mute);
    if (list->cur_pos >= list->list_num) {
      item = NULL;
    } else {
      item = &(list->list[list->cur_pos]);
      list->cur_pos++;
    }
    if (list->cur_pos > 200) {
      list->cur_pos = 0;
    }
    param->thread_ids[thread_id] = -1;
    pthread_mutex_unlock(&mute);
    if (item == NULL) {
      break;
    }

    int sockfd = webSocket_clientLinkToServer(param->ip, param->port, "/null");
    if (sockfd < 0) {
      printf("Cannot connect websocket server %s %d\n", param->ip, param->port);
      exit(-1);
      return NULL;
    }
    int nZero = 20 * 1024 *1024 ;
    setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,(char *)&nZero,sizeof(int));

    Setnonblocking(sockfd);

    UserData uu;
    uu.item = item;
    uu.fd = sockfd;
    uu.thread_id = thread_id;

    struct epoll_event event;
    event.data.ptr = &uu;
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;

    if (0 != epoll_ctl(param->epfd, EPOLL_CTL_ADD, sockfd, &event)) {
      perror("epoll_ctl");
      exit(-1);
      return NULL;
    }

    FILE *fp = fopen(item->path, "r");
    int rc = 0;
    char buf[20480];

    while ( (rc = fread(buf, sizeof(char), 20480, fp)) > 0 ) {
      int n = webSocket_send(sockfd, buf, rc, true, WDT_TXTDATA);
      //printf("socket %d write to server[ret = %d]\n", sockfd, n);
    }
    buf[0] = 0;
    strcat(buf, "EOS");
    int n = webSocket_send(sockfd, buf, 3, true, WDT_TXTDATA);
    //printf("socket %d write to server[ret = %d]\n", sockfd, n);
    fclose(fp);
    pthread_mutex_lock(&mute); 
    while (param->thread_ids[thread_id] != 1) {
      pthread_cond_wait(&cond, &mute);
    }
    param->thread_ids[thread_id] = -1;
    pthread_mutex_unlock(&mute); 
    epoll_ctl(param->epfd, EPOLL_CTL_DEL, sockfd, &event);
    close(sockfd);
  }
  return NULL;
}

FileList *load_list(const char *list_name) {
  FILE *fp = fopen(list_name, "r");
  vector<string> ids;
  vector<string> paths;
  char line[2014];
  while (!feof(fp)){
    memset(line, 0, sizeof(line));
    fgets(line, sizeof(line) - 1, fp); // 包含了换行符
    printf("%s", line);
    char id[256] = {0};
    char path[1024] = {0};
    sscanf(line, "%s\t%s\n", id, path);
    ids.push_back(string(id));
    paths.push_back(string(path));
  }
  fclose(fp);
  FileList *list = (FileList*)malloc(sizeof(FileList));
  list->list = (FileItem*)malloc(sizeof(FileItem) * ids.size());
  list->list_num = ids.size();
  list->cur_pos = 0;
  for (int i = 0; i < ids.size(); i++) {
    list->list[i].id[0] = 0;
    list->list[i].path[0] = 0;
    strcat(list->list[i].id, ids[i].c_str());
    strcat(list->list[i].path, paths[i].c_str());
  }
  return list;
}

void free_list(FileList *list) {
  if (list->list) {
    free(list->list);
  }
  if (list) {
    free(list);
  }
}
int main(int argc, char **argv) {

  if (argc != 5) {
    printf("usage: %s <IPaddress> <PORT> <thread-num> <file.list>\n", argv[0]);
    return 1;
  }
  SetRLimit();
  //printf("FD_SETSIZE= %d\n", FD_SETSIZE);

  pthread_mutex_init(&mute, NULL);
  pthread_cond_init(&cond, NULL);

  int epfd = epoll_create(512);

  if (epfd < 0) {
    perror("epoll_create");
    return 1;
  }

  Param param;
  memset(&param, 0, sizeof(param));
  strcat(param.ip, argv[1]);
  param.port = atoi(argv[2]);
  int threadNum = atoi(argv[3]);
  param.list = load_list(argv[4]);
  param.epfd = epfd;
  param.thread_ids = new int[threadNum];

  struct epoll_event ev[20480];

  pthread_t *thread = new pthread_t[threadNum];
  for (int i = 0; i < threadNum; i++) {
    param.thread_id = i;
    if (0 != CreateThread(SendThread, (void *)&param, &(thread[i]))) {
      perror("CreateThread");
      return -1;
    }
    usleep(1000);
    //printf("thread %d\n", int(thread[i]));
  }

  int nfds = 0; 

  while (1) {
    nfds = epoll_wait(epfd, ev, 20480, -1);
    if (nfds < 0) {
      perror("epoll_wait");
      break;
    } else if (nfds == 0) {
      printf("epoll_wait timeout!\n");
      continue;
    }
    for (int i = 0; i < nfds; i++) {
      if (ev[i].events & EPOLLIN) {
        UserData *uu = (UserData*)ev[i].data.ptr;
        //printf("can read for %d now\n", uu->fd);
        char data[1024] = {0};
        int n = webSocket_recv(uu->fd, data, sizeof(data), NULL);
        //printf("Received %d bytes from server(%s)!\n", n, data);
        if (uu != NULL && n > 0) {
          FileItem *item = uu->item;
          printf("id:%s\t%s\n", item->id, data);
        }
        if (false && n <= 0) {
          //printf("end!!!!\n");
          pthread_cond_broadcast(&cond); 
        }
      }
      if (ev[i].events & EPOLLRDHUP) {
        UserData *uu = (UserData*)ev[i].data.ptr;
        //printf("can read for %d now hub\n", uu->fd);
        //printf("end!!!!hub\n");
        pthread_mutex_lock(&mute);
        param.thread_ids[uu->thread_id] = 1; 
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&mute);
      }
    }
  }

  for (int i = 0; i < threadNum; i++) {
    pthread_join(thread[i], NULL);
  }
  pthread_cond_destroy(&cond);
  pthread_mutex_destroy(&mute);
  delete [] thread;
  free_list(param.list);
  close(epfd);
  return 0;
}
