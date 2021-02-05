#include "client.h"
int main(int argc,const char* argv[])
{
	int connfd,ret;
	//判断命令行输入个数
	if(argc < 3){
		printf("example:%s ip port\n",argv[0]);
		exit(1);
	}
	//判断命令行输入是否正确
	char ip[16] = "";
	strcpy(ip,argv[1]);
	int port = atoi(argv[2]);
	ret = check_argv(ip, port);
	if(ret == -1)
		exit(1);
	//初始化套接字
	connfd = cli_init(ip,port);
	if(connfd == -1)
		exit(1);
	//客户端登录
	view_login(connfd);
	return 0;
}
