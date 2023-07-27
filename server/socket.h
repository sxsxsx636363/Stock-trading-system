#ifndef SOCKET_H
#define SOCKET_H
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <arpa/inet.h>
#include <memory.h>
#include <unistd.h>
#include "exception.h"
#define BACKLOG 100
#define SERVERPORT "12345"
using namespace std;

int startServer();
int acceptConnection(int socket_fd);
int startClient(const char * hostname, const char * port);
void closefd(int socket_fd);

#endif
