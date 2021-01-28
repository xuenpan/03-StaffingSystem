#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,const char* argv[])
{
	if(argc < 3){
		printf("example:%s ip port\n",argv[0]);
		exit(1);
	}
	//判断输入格是否正确
	char ip[20];
	int port = atoi(argv[2]);
	strcpy(ip,argv[1]);

	int sockfd,connfd,prot,ret;
	//创建客户端套接字
	connfd = socket(AF_INET,SOCK_STREAM,0);
	if(connfd == -1){
		perror("socket");
		return -1;
	}
	//填充服务器结构信息
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port   = htons(port);
	sin.sin_addr.s_addr = inet_addr(ip);
	socklen_t sinlen = sizeof(sin);
	//连接服务器
	ret = connect(connfd,(struct sockaddr*)&sin,sinlen);
	if(ret == -1){
		perror("connec");
		return -1;
	}
	while(1);
	return 0;
}
