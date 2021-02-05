#include "server.h"
/*创建并初始化数据库*/
sqlite3* create_table(void)
{
	//创建数据库
	sqlite3* db    = NULL;
	char* errmsg   = NULL;
	char** resultp = NULL; //结果
	int nrow       = -1; //查找出的总行数
	int ncolumn    = -1; //列
	char* sql      = NULL;
	//打开数据库
	if(sqlite3_open("./table/usr.db",&db) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,sqlite3_errmsg(db));
		return NULL;
	}
	sql = "drop table Login";
	//服务器重启时删除登录表
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		//return NULL;
	}
	printf("Ready1:drop Login table ok!\n");
	//创建员工注册表
	//用户名,密码,工号,姓名,生日,等级,部门,职位,薪资,电话,住址
	sql = "create table if not exists Register(username char primary key,\
		   passwd char,id int,name char,birthday char,level int,department char,\
		   post char,salary int,telephone char,address char)";
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		return NULL;
	}
	printf("Ready2:Register table ok!\n");
	//在注册表中查询admin是否存在,存在则不添加
	sql = "select * from Register where username='admin'";
	if(sqlite3_get_table(db,sql,&resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		return NULL;
	}
	//如果行数大于0则表示注册表中存在admin用户
	if(nrow > 0){
		fprintf(stderr,"admin exist in Register!\n");
	}else{
		//部分插入管理员用户名admin密码12345
		sql = "insert into Register values('admin','12345',1,'管理员','19700101',1,'x','x',1,'xxx','xxx')";
		if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		}
	}
	printf("Ready3:Register have admin!\n");
	//创建员工登录表
	//用户名,套接字,(套接字用来客户端未正常退出的时候做删除使用)
	sql = "create table Login(username char primary key,acceptfd int)";
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		return NULL;
	}
	printf("Ready4:Login table ok!\n");
	//创建员工考勤表
	sql = "create table if not exists Attendance(username char,\
		   login char,logout char)";
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		sqlite3_close(db);
		return NULL;
	}
	printf("Ready5:Attendance table ok!\n");
	return db;
}
/*初始化套接字*/
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
	fprintf(stderr,"Listen ready......\n\n");
	return sockfd;
}
/********上面部分为初始化内容**********/
/*分线程处理多客户端信息*/
void *ser_docli(void* arg){
	//当线程结束的时候自动回收资源
	pthread_detach(pthread_self());
	//保存结构体
	msg_ser msg_s = *((msg_ser*)arg);
	msg_cli msg;
	while(1){
		//因为每次接收消息都用到,所以每次都先清空
		bzero(&msg,sizeof(msg));

		if(ser_recv(&msg_s,&msg,sizeof(msg)) == -1){
			fprintf(stderr,"Client[%d] quit\n\n",msg_s.acceptfd);
			//接收失败则表示有错误，该线程退出
			pthread_exit(NULL);
		}
		switch (msg.type){
		case LOGIN: //登录
			cli_login(&msg_s,&msg,sizeof(msg));
			break;
		case EXIST://查看用户是否存在
			cli_exist(&msg_s,&msg,sizeof(msg));
			break;
		case ADDUSER: //添加用户
			cli_adduser(&msg_s,&msg,sizeof(msg));
			break;
		case ALLUSER: //查看所有用户名
			cli_alluser(&msg_s,&msg,sizeof(msg));
			break;
		case DELUSER: //删除用户
			cli_deluser(&msg_s,&msg,sizeof(msg));
			break;
		case VIEWMSG: //查看用户信息
			cli_viewmsg(&msg_s,&msg,sizeof(msg));
			break;
		case CHANMSG: //修改信息
			cli_changemsg(&msg_s,&msg,sizeof(msg));
			break;
		case ADDLOG: //添加考勤
			cli_attendance(&msg_s,&msg,sizeof(msg));
			break;
		case LOG: //查看考勤
			cli_log(&msg_s,&msg,sizeof(msg));
			break;
		case QUIT: //退出
			cli_quit(&msg_s,&msg,sizeof(msg));
			break;
		default:
			//说明接收有问题
			printf("%d:Recv error type!\n",__LINE__);
			close(msg_s.acceptfd);
			pthread_exit(NULL);
		}
	}
}
/*客户端登录*/
int cli_login(msg_ser* msg_s,msg_cli* msg,int len)
{
	char sql[256]  = "";
	char* errmsg   = NULL;
	char** resultp = NULL;
	int nrow       = -1; //查找出的总行数
	int ncolumn    = -1; //存储列
	int ret        = -1;
	ret = strcmp(msg->username,"admin");
	if(ret == 0){ //管理员登录
		//到注册表中对比一下密码
		sprintf(sql,"select * from Register where username='%s' and passwd='%s'",\
				msg->username,msg->passwd);
		if(sqlite3_get_table(msg_s->db,(const char*)sql,&resultp,&nrow,&ncolumn,&errmsg)\
				!= SQLITE_OK){
			fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
			return -1;
		}
		//密码匹配成功查看登录表是否重复登录
		if(nrow > 0){ //在注册表中对应上密码
			//到登录表中查看是否重复登录
			sprintf(sql,"insert into Login values('%s',%d)",msg->username,msg_s->acceptfd);
			if(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
				fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
				//插入失败则表示重复登录
				msg->type = REPEAT;
				ser_send(msg_s,msg,len);
				return -1;
			}
			msg->type = ADMIN;
			ser_send(msg_s,msg,len);
		}else{ //在注册表中没有匹配上密码
			//发送密码错误
			msg->type = PWERR;
			ser_send(msg_s,msg,len);
			return -1;
		}
	}else{ //普通员工登录
		//到注册表中匹配用户名
		sprintf(sql,"select * from Register where username='%s'",msg->username);
		if(sqlite3_get_table(msg_s->db,(const char*)sql,&resultp,&nrow,&ncolumn,&errmsg)\
				!= SQLITE_OK){
			fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
			return -1;
		}
		if(nrow > 0){ //查找到用户名说明用户存在
			bzero(sql,sizeof(sql));
			sprintf(sql,"select * from Register where username='%s' and passwd='%s'",\
					msg->username,msg->passwd);
			if(sqlite3_get_table(msg_s->db,(const char*)sql,&resultp,&nrow,&ncolumn,&errmsg)\
					!= SQLITE_OK){
				fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
				return -1;
			}
			//密码匹配成功查看登录表是否重复登录
			if(nrow > 0){
				//到登录表中查看是否重复登录
				sprintf(sql,"insert into Login values('%s',%d)",msg->username,msg_s->acceptfd);
				if(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
					//插入失败则表示重复登录
					msg->type = REPEAT;
					ser_send(msg_s,msg,len);
					return -1;
				}
				msg->type = OTHER;
				ser_send(msg_s,msg,len);
			}else{ //在注册表中没有匹配上密码
				//发送密码错误
				msg->type = PWERR;
				ser_send(msg_s,msg,len);
			}

		}else{
			msg->type = UNFIND;
			ser_send(msg_s,msg,len);
		}
	}
	return 0;
}
/*客户端查看用户是否存在*/
int cli_exist(msg_ser* msg_s,msg_cli* msg,int len)
{
	char sql[128]  = "";
	char* errmsg   = NULL;
	char** resultp = NULL;
	int nrow       = -1;
	int ncolumn    = -1;
	sprintf(sql,"select * from Register where username='%s'",msg->data);
	if(sqlite3_get_table(msg_s->db,sql,&resultp,&nrow,&ncolumn,&errmsg)\
			!= SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		return -1;
	}
	if(nrow > 0){//用户存在
		strcpy(msg->data,"exist");
		ser_send(msg_s,msg,len);	
	}else{ //用户不存在
		strcpy(msg->data,"NoExist");
		ser_send(msg_s,msg,len);
	}
	return 0;
}
/*客户端管理员添加用户*/
int cli_adduser(msg_ser* msg_s,msg_cli* msg,int len)
{
	char sql[128] = "";
	char* errmsg  = NULL;
	sprintf(sql,"insert into Register values('%s','%s',%d,'%s','%s',%d,'%s','%s',%d,'%s','%s')",\
			msg->data,msg->passwd,msg->staff.id,msg->staff.name,msg->staff.birthday,\
			msg->staff.level,msg->staff.department,msg->staff.post,msg->staff.salary,\
			msg->staff.telephone,msg->staff.address);
	//插入后发过去没用,清空信息结构体
	bzero(&(msg->staff),sizeof(msg->staff));
	if(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		ser_send(msg_s,msg,len);
		return -1;
	}
	printf("insert %s into Register succeed!\n",msg->data);
	strcpy(msg->data,"OK");
	if(ser_send(msg_s,msg,len) == -1)
		return -1;
	return 0;
}
/*客户端管理员查看所有用户名*/
int cli_alluser(msg_ser* msg_s,msg_cli* msg,int len)
{
	char sql[128]  = "";
	char* errmsg   = NULL;
	char** resultp = NULL;
	int nrow       = -1;
	int ncolumn    = -1;

	//查询登录表中的用户名
	sprintf(sql,"select username,name from Register");
	if(sqlite3_get_table(msg_s->db,(const char*)sql,&resultp,&nrow,&ncolumn,&errmsg)\
			!= SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		return -1;
	}
	int i,j,index=2;
	bzero(msg->data,sizeof(msg->data));
	for(i=1; i<=nrow; i++){
		for(j=0; j<ncolumn; j++){
			strcat(msg->data,resultp[index++]);
			strcat(msg->data,"  ");
		}
		ser_send(msg_s,msg,len);
		//清零,以防数据长度不一致
		bzero(msg->data,sizeof(msg->data));
	}
	bzero(msg->data,sizeof(msg->data));
	strcpy(msg->data,"FindEnd");
	ser_send(msg_s,msg,len);

	return 0;
}
/*客户端管理员删除用户*/
int cli_deluser(msg_ser* msg_s,msg_cli* msg,int len)
{
	char sql[128] = "";
	char* errmsg  = NULL;

	sprintf(sql,"delete from Register where username='%s'",msg->data);
	while(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		//删除失败表明用户名输入错误
		strcpy(msg->data,"username error");
		ser_send(msg_s,msg,len);
		return -1;
	}
	strcpy(msg->data,"OK");
	ser_send(msg_s,msg,len);
	return 0;
}
/*客户端查看用户*/
int cli_viewmsg(msg_ser* msg_s,msg_cli* msg,int len)
{
	char sql[128]  = "";
	char* errmsg   = NULL;
	char** resultp = NULL;
	int nrow       = -1;
	int ncolumn    = -1;
	sprintf(sql,"select * from Register where(username='%s')",msg->data);
	if(sqlite3_get_table(msg_s->db,sql,&resultp,&nrow,&ncolumn,&errmsg)\
			!= SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		return -1;
	}
	//找到用户名
	if(nrow > 0){
		int i,index=11;
		for(i=0; i<ncolumn; i++){
			strcpy(msg->data,resultp[index++]);
			ser_send(msg_s,msg,len);
		}
		strcpy(msg->data,"end");
		ser_send(msg_s,msg,len);
	}else{ //未找到用户名
		strcpy(msg->data,"NoFind");
		ser_send(msg_s,msg,len);
	}
	return 0;
}
/*客户端修改信息*/
int cli_changemsg(msg_ser* msg_s,msg_cli* msg,int len)
{
	char sql[128] = "";
	char* errmsg  = NULL;
	//区分管理员和普通用户
	if(strcmp(msg->username,"admin") == 0){
		//先删除对应用户信息
		sprintf(sql,"delete from Register where username='%s'",msg->data);
		if(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
			return -1;
		}
		printf("delete %s in Register succeed!\n",msg->data);
		//重新插入
		cli_adduser(msg_s,msg,len);
	}else{
		sprintf(sql,"update Register set passwd='%s',birthday='%s',telephone='%s',address='%s' where username='%s'",\
				msg->passwd,msg->staff.birthday,msg->staff.telephone,msg->staff.address,msg->username);
		if(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
			return -1;
		}
		printf("update %s in Register succeed\n",msg->username);
		strcpy(msg->data,"OK");
		ser_send(msg_s,msg,len);
	}
	return 0;
}
/*客户端其他用户添加考勤*/
int cli_attendance(msg_ser* msg_s,msg_cli* msg,int len)
{
	char sql[128] = "";
	char* errmsg  = NULL;
	time_t tm = time(NULL);

	//从data判断是上班打卡还是下班打卡
	if(strcmp(msg->data,"login") == 0){//上班打卡
		sprintf(sql,"insert into Attendance(username,login) values ('%s','%s')",\
				msg->username,ctime(&tm));
		if(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
			return -1;
		}
	}else{
		sprintf(sql,"insert into Attendance(username,logout) values ('%s','%s')",\
				msg->username,ctime(&tm));
		if(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
			fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
			return -1;
		}
	}
	strcpy(msg->data,"OK");
	ser_send(msg_s,msg,len);
	return 0;
}
/*客户端查看考勤*/
int cli_log(msg_ser* msg_s,msg_cli* msg,int len)
{
	char username[20] = "";
	char sql[128]     = "";
	char* errmsg      = NULL;
	char** resultp    = NULL;
	int nrow          = -1;
	int ncolumn       = -1;
	int i=0, index=1;

	//保存一下data中的用户名
	strcpy(username,msg->data);
	//先发送上班打卡
	sprintf(sql,"select login from Attendance where username='%s'",username);
	if(sqlite3_get_table(msg_s->db,sql,&resultp,&nrow,&ncolumn,&errmsg) \
			!= SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		return -1;
	}
	if(nrow > 0){ //找到该用户名登录信息
		for(i=1; i<=nrow; i++){
			if(resultp[index] == NULL);
			else{
				strcpy(msg->data,resultp[index]);
				printf("%d****msg->data:%s\n",__LINE__,msg->data);
				ser_send(msg_s,msg,len);
			}
			index++;
		}
		strcpy(msg->data,"end");
		ser_send(msg_s,msg,len);
	}else{ //未找到该用户
		strcpy(msg->data,"NoFind");
		ser_send(msg_s,msg,len);
	}

	index = 1;
	//再发送下班打卡
	sprintf(sql,"select logout from Attendance where username='%s'",username);
	if(sqlite3_get_table(msg_s->db,sql,&resultp,&nrow,&ncolumn,&errmsg) \
			!= SQLITE_OK){
		fprintf(stderr,"%d:%s\n",__LINE__,errmsg);
		return -1;
	}
	if(nrow > 0){ //找到该用户名登录信息
		for(i=1; i<=nrow; i++){
			if(resultp[index] == NULL);
			else{
				strcpy(msg->data,resultp[index]);
				ser_send(msg_s,msg,len);
			}
			index++;
		}
		strcpy(msg->data,"end");
		ser_send(msg_s,msg,len);
	}else{ //未找到该用户
		strcpy(msg->data,"NoFind");
		ser_send(msg_s,msg,len);
	}
	return 0;
}
/*客户端退出*/
int cli_quit(msg_ser* msg_s,msg_cli* msg,int len)
{
	char sql[128] = "";
	char* errmsg = NULL;
	sprintf(sql,"delete from Login where username='%s'",msg->username);
	if(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		fprintf(stderr,"%d:%s:%s\n",__LINE__,__func__,errmsg);
		return -1;
	}
	printf("username[%s] quit delte in Login table!\n\n",msg->username);
	return 0;
}

//接收客户端消息
int ser_recv(msg_ser* msg_s, msg_cli* msg, int len)
{
	bzero(msg,len);
	int ret = recv(msg_s->acceptfd,(void*)msg,len,0);
	if(ret <= 0){
		if(0 == ret){
			//客户端未正常退出,在登录表中删除该用户
			char sql[128] = "";
			char* errmsg = NULL;
			sprintf(sql,"delete from Login where acceptfd=%d",msg_s->acceptfd);
			if(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
				fprintf(stderr,"%d:%s:%s\n",__LINE__,__func__,errmsg);
				close(msg_s->acceptfd);
				return -1;
			}
			printf("Client[%d] quit delte in Login table!\n",msg_s->acceptfd);
			//关闭套接字，避免造成浪费
			close(msg_s->acceptfd);
			return -1;
		}
		perror("recv");
		close(msg_s->acceptfd);
		return -1;
	}
	//接收成功打印一下
	printf("Recv: Client[%d] type:%2d username:%s passwd:%s data:%s\n",\
			msg_s->acceptfd,msg->type,msg->username,msg->passwd,msg->data);
	return 0;
}
//发送消息给客户端
int ser_send(msg_ser* msg_s,msg_cli* msg,int len)
{
	int ret = send(msg_s->acceptfd,(const void*)msg,len,0);
	if(ret <= 0){
		if(0 == ret){
			//客户端未正常退出,在登录表中删除该用户
			char sql[128] = "";
			char* errmsg = NULL;
			sprintf(sql,"delete from Login where acceptfd=%d",msg_s->acceptfd);
			if(sqlite3_exec(msg_s->db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
				fprintf(stderr,"%d:%s:%s\n",__LINE__,__func__,errmsg);
				close(msg_s->acceptfd);
				return -1;
			}
			printf("Client[%d] quit Delte in Login table!\n",msg_s->acceptfd);
			//关闭套接字，避免造成浪费
			close(msg_s->acceptfd);
			return -1;
		}
		perror("recv");
		close(msg_s->acceptfd);
		return -1;
	}
	//发送成功打印一下
	printf("Send: Client[%d] type:%2d username:%s passwd:%s data:%s\n\n",\
			msg_s->acceptfd,msg->type,msg->username,msg->passwd,msg->data);
	return 0;
}
