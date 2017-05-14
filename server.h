#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>//有memset()的定义

#define PORT  8888

int server_up();

#endif