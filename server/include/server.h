#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <sqlite3.h>
#include <string.h>

#define SER_PORT 6666
#define SER_IP   "192.168.31.227"

//用来存储分线程需要的信息
typedef struct server_msg{
	sqlite3 *db;
	int acceptfd;
	char data[128];//这个暂时不知道用来干嘛
}__attribute__((packed)) msg_ser;

struct message{
	int id;
	char name[20];
	int brithday;
	int level;
	char department[50];
	char post[50];
	int salary;
	int telephone;
	char address[50];
}__attribute__((packed));

typedef struct client_msg{
	char type;
	char username[20];
	char passwd[20];
	struct message staff;
}__attribute__((packed)) msg_cli;

sqlite3* create_table(void);
int ser_init(void);
void *ser_docli(void* arg);


#endif
