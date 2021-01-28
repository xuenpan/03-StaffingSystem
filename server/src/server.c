#include "server.h"
//创建并初始化数据库
sqlite3* create_table(void)
{
	//创建数据库
	sqlite3* db = NULL;
	//打开数据库
	if(sqlite3_open("./table/usr.db",&db) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,sqlite3_errmsg(db));
		return NULL;
	}
	char *sql    = "drop table Login";
	char* errmsg = NULL;
	//服务器重启时删除登录表
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		//删除登录表失败则关闭数据库,主程序需退出
		//sqlite3_close(db);
		//return NULL;
	}
	printf("Ready1:drop Login table ok!\n");
	//创建员工注册表
	//用户名,密码,工号,姓名,生日,等级,部门,职位,薪资,电话,住址
	sql = "create table if not exists Register(username char primary key,\
		   passwd char,id int,name char,brithday int,level int,department char,\
		   post char,salary int,telephone int,address char)";
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		sqlite3_close(db);
		return NULL;
	}
	printf("Ready2:Register table ok!\n");
	//创建员工登录表
	//用户名,密码
	sql = "create table Login(name char primary key,passwd char)";
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		sqlite3_close(db);
		return NULL;
	}
	printf("Ready3:Login table ok!\n");
	//创建员工考勤表
	//这里应该外接打卡机用打卡机的数据,暂时用登录时间及登出时间代替
	sql = "create table if not exists Attendance(login char,logout char)";
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		sqlite3_close(db);
		return NULL;
	}
	printf("Ready4:Attendance table ok!\n");
	return db;
}
//初始化套接字
int ser_init(void)
{
	int sockfd,ret;
	struct sockaddr_in sin;
	socklen_t sinlen,cinlen;
	//创建套接字
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1){
		perror("socket");
		return -1;
	}
	fprintf(stderr,"create socket succeed!\n");
	//允许地址快速重用
	int b_reuse = 1;
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&b_reuse,sizeof(int))!=0){
		perror("setsockopt");
		return -1;
	}
	//填充服务器结构体信息 man 7 ip
	sin.sin_family      = AF_INET;
	sin.sin_port        = htons(SER_PORT);
	sin.sin_addr.s_addr = inet_addr(SER_IP);
	sinlen = sizeof(sin);

	//绑定服务器结构体信息
	if((ret = bind(sockfd,(struct sockaddr*)&sin,sinlen)) == -1){
		perror("bind");
		return -1;
	}
	fprintf(stderr,"socket bind succeed!\n");

	//监听,把自动套接字改为被动套接字
	if((ret = listen(sockfd,20)) == -1){
		perror("listen");
		return -1;
	}
	fprintf(stderr,"listen ready......\n");
	return sockfd;
}
void *ser_docli(void* arg){
	//当线程结束的时候自动回收资源
	pthread_detach(pthread_self());
	//保存结构体
	msg_ser msg_s = *((msg_ser*)arg);
	msg_cli msg;
	bzero(&msg,sizeof(msg));
}
