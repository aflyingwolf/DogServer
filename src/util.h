
/*
@author Jiang W
@time   2020.6.14
@email  aflyingwolf@126.com
**/

#pragma once
#include <uuid/uuid.h>
#include <string>
namespace server {
int socket_bind_listen(int port);
int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);
std::string gen_uuid();

}

