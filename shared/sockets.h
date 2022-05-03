#ifndef SRC_CONNECT_H_
#define SRC_CONNECT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>


#define BACKLOG 10

int socket_create_listener(char* ip, char* port);
int socket_connect_to_server(char* ip, char* port);


#endif /* SRC_CONNECT_H_ */
