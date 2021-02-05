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
#include <time.h>

#define SER_PORT 6666
#define SER_IP   "192.168.1.9"

#define LOGIN   1  //登录
#define ADDUSER 2  //添加用户
#define DELUSER 3  //删除用户
#define CHANMSG 4  //修改信息
#define VIEWMSG 5  //查看信息
#define LOG     6  //考勤
#define QUIT    7  //退出
#define ADMIN   8  //管理员
#define OTHER   9  //其他用户
#define REPEAT  10 //重复
#define PWERR   11 //密码错误
#define UNFIND  12 //未找到
#define ALLUSER 13 //所有用户
#define EXIST   14 //用户存在
#define ADDLOG  15 //添加考勤

//存储分线程需要的信息
typedef struct server_msg{
	sqlite3 *db;
	int acceptfd;
}__attribute__((packed)) msg_ser;
//存储员工的信息
struct message{
	int  id;             //工号
	char name[20];       //姓名
	char birthday[10];   //生日
	int  level;          //等级
	char department[50]; //部门
	char post[50];       //职位
	int  salary;         //薪水
	char telephone[13];  //电话
	char address[50];    //住址
}__attribute__((packed));
//与客户端通信结构体
typedef struct client_msg{
	int  type;           //类型
	char username[20];   //用户名
	char passwd[20];     //密码
	char data[20];       //数据
	struct message staff;//员工信息结构体
}__attribute__((packed)) msg_cli;

sqlite3* create_table(void);
int ser_init(void);

void *ser_docli(void* arg);
int cli_login(msg_ser* msg_s,msg_cli* msg,int len);
int cli_exist(msg_ser* msg_s,msg_cli* msg,int len);
int cli_adduser(msg_ser* msg_s,msg_cli* msg,int len);
int cli_alluser(msg_ser* msg_s,msg_cli* msg,int len);
int cli_deluser(msg_ser* msg_s,msg_cli* msg,int len);

int cli_viewmsg(msg_ser* msg_s,msg_cli* msg,int len);
int cli_changemsg(msg_ser* msg_s,msg_cli* msg,int len);
int cli_attendance(msg_ser* msg_s,msg_cli* msg,int len);
int cli_log(msg_ser* msg_s,msg_cli* msg,int len);

int cli_quit(msg_ser* msg_s,msg_cli* msg,int len);

int ser_recv(msg_ser* msg_s,msg_cli* msg,int len);
int ser_send(msg_ser* msg_s,msg_cli* msg,int len);

#endif
