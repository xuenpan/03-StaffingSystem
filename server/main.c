#include "server.h"

int main(int argc, const char *argv[])
{
	int sockfd,acceptfd,ret;
	struct sockaddr_in cin;
	msg_ser msg_s;
	socklen_t cinlen = sizeof(cin);
	//数据库准备
	msg_s.db = create_table();
	if(msg_s.db == NULL){
		fprintf(stderr,"%d:sqlite3 error!\n",__LINE__);
		return -1;
	}
	//套接字准备
	sockfd = ser_init();
	if(sockfd == -1){
		fprintf(stderr,"%d:server quit!\n\n",__LINE__);
		return -1;
	}
	while(1){
		//阻塞等待与客户端建立连接
		acceptfd = accept(sockfd,(struct sockaddr*)&cin,&cinlen);
		if(-1 == acceptfd){
			if(errno == EINTR) //避免被信号干扰,没用到信号其实没啥用
				continue;
			perror("accept");
			exit(1);
		}
		fprintf(stderr,"accpet:%d\n",acceptfd);
		//将线程套接字给到
		msg_s.acceptfd = acceptfd;
		pthread_t tid;
		pthread_create(&tid,NULL,ser_docli,(void*)&msg_s);
	}
	//服务器退出需关闭套接字及数据库
	close(sockfd);
	sqlite3_close(msg_s.db);
	return 0;
}
