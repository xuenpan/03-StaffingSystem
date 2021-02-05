#ifndef __CLIENT_H__
#define __CLIENT_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
//#include <stdlib.h>

#define LOGIN   1
#define ADDUSER 2
#define DELUSER 3
#define CHANMSG 4
#define VIEWMSG 5
#define LOG     6
#define QUIT    7
#define ADMIN   8
#define OTHER   9
#define REPEAT  10
#define PWERR   11
#define UNFIND  12
#define ALLUSER 13
#define EXIST   14
#define ADDLOG  15

int pwerr;
//储存员工信息
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
//与服务器进行通信结构体
typedef struct client_msg{
	int  type;           //类型
	char username[20];   //用户名
	char passwd[20];     //密码
	char data[20];       //数据
	struct message staff;//员工信息结构体
}__attribute__((packed)) msg_cli;

int check_argv(char* ip,int port);
int cli_init(char* ip,int port);

void view_login(int connfd);
int cli_login(int connfd);
int cli_login_judge(int connfd,msg_cli* msg,int len);
int do_admin(int connfd,msg_cli* msg,int len);
int do_other(int connfd,msg_cli* msg,int len);
void do_admin_view(void);
void do_other_view(void);

int admin_add(int connfd,msg_cli* msg,int len);
int admin_alluser(int connfd,msg_cli* msg,int len);
int admin_del(int connfd,msg_cli* msg,int len);

int view_msg(int connfd,msg_cli* msg,int len);
int input_msg(msg_cli* msg);
int change_msg_admin(int connfd,msg_cli* msg,int len);
int view_log(int connfd,msg_cli* msg,int len);
int quit_login(int connfd,msg_cli* msg,int len);

int change_msg_other(int connfd,msg_cli* msg,int len);
int other_attendance(int connfd,msg_cli* msg,int len);

int cli_send(int connfd,msg_cli* msg,int len);
int cli_recv(int connfd,msg_cli* msg,int len);

#endif
