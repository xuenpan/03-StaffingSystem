#include "client.h"
/*判断输入是否正确*/
int check_argv(char* ip,int port)
{
	int ret, i=0, temp=0;
	char cup[4] = "";
	//判断ip输入是否正确
	while(1){
		if(*ip >= '0' && *ip <= '9'){
			cup[i] = *ip;
			i++;
		}else if(*ip == '.'){
			ret = atoi(cup);
			if(ret<0 || ret>255){
				printf("ip values input error!\n");
				return -1;
			}
			bzero(cup,sizeof(cup));
			i=0;
			temp++;
		}else if(temp == 3){
			ret = atoi(cup);
			if(ret<0 || ret>255){
				printf("ip values input error!\n");
				return -1;
			}
			break;	
		}else if(*ip == '\0'){
			break;
		}else if(*ip < '0' || *ip > '9'){
			printf("ip values input error!\n");
			return  -1;
		}
		ip++;
	}
	if(temp != 3){
		printf("ip input error(xxx.xxx.xxx)!\n");
		return -1;
	}
	//判断port输入是否正确
	if(port<1024 || port>65535){
		printf("port input error(1024-65535)\n");
		return -1;
	}
	return 0;
}
/*初始化套接字*/
int cli_init(char* ip,int port)
{
	int connfd,ret;

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
		perror("connect");
		return -1;
	}
	return connfd;
}
/*一级登录界面*/
void view_login(int connfd)
{
	int ret;
	while(1){
		system("clear");
		printf("\n");
		printf(" ┏━┓━━━━━━━━━━━━━━━━━━━━━┓━┓\n");
		printf(" ┃ ┃    员工管理系统     ┃ ┃\n");
		printf(" ┃ ┗━━━━━━━━━━━━━━━━━━━━━┛ ┃\n");
		printf(" ┃ ┃[1]登录              ┃ ┃\n");
		printf(" ┃ ┃                     ┃ ┃\n");
		printf(" ┃ ┃[2]退出              ┃ ┃\n");
		printf(" ┃ ┃                     ┃ ┃\n");
		printf(" ┗━┗━━━━━━━━━━━━━━━━━━━━━┛━┛\n");
		printf("Please input your choose -> ");
		fflush(stdout);
		scanf("%d",&ret);
		//吃掉'\n'
		while(getchar() != '\n');
		switch(ret){
		case 1:
			cli_login(connfd);
			break;
		case 2:
			return;
			break;
		default:
			printf("input error(press enter key to continue)");
			fflush(stdout);
			while(getchar() != '\n');
		}
	}
}
/*登录:用户输入账户和密码*/
int cli_login(int connfd)
{
	int ret;
	msg_cli msg;
	bzero(&msg,sizeof(msg));
	//设置类型为登录
	msg.type = LOGIN;
	//账号
	printf("Please input your username -> ");
	fflush(stdout);
	fgets(msg.username,sizeof(msg.username),stdin);
	//将最后的'\n'置为'\0'
	msg.username[strlen(msg.username)-1] = '\0';
	//密码
	printf("please input your password -> ");
	fflush(stdout);
	fgets(msg.passwd,sizeof(msg.passwd),stdin);
	//将最后的'\n'置为'\0'
	msg.passwd[strlen(msg.passwd)-1] = '\0';
	//发给服务器确认
	ret = cli_send(connfd,&msg,sizeof(msg));
	if(ret == -1)
		return -1;
	//接收服务器消息
	ret = cli_recv(connfd,&msg,sizeof(msg));
	if(ret == -1)
		return -1;
	ret = cli_login_judge(connfd,&msg,sizeof(msg));
	if(ret == -1)
		return -1;
	return 0;
}
/*判断管理员和普通用户，密码正确与否，是否重复登录*/
int cli_login_judge(int connfd,msg_cli* msg,int len)
{
	//如果服务器type返回值区分
	switch(msg->type){
	case ADMIN:
		do_admin(connfd,msg,len);
		break;
	case OTHER:
		do_other(connfd,msg,len);
		break;
	case REPEAT:
		printf("重复登录!(按Enter键继续)");
		fflush(stdout);
		while(getchar() != '\n');
		return -1;
	case UNFIND:
		printf("用户名错误!(按Enter键继续)");
		fflush(stdout);
		while(getchar() != '\n');
		break;
	case PWERR:
		if(pwerr < 2){
			pwerr += 1;
			if(pwerr == 1)
				printf("密码错误(剩下两次机会)！\n");
			else if(pwerr == 2)
				printf("密码错误(剩下一次机会)！\n");
			cli_login(connfd);
		}
		else
			//输错三次,进程退出
			exit(1);
		break;
	default:
		printf("接收服务器消息错误！\n");
		return -1;
	}
	return 0;
}
/*管理员操作*/
int do_admin(int connfd,msg_cli* msg,int len){
	int ret;
	while(1){
		do_admin_view();
		//输入选择
		printf("Please input your choose -> ");
		fflush(stdout);
		scanf("%d",&ret);
		while(getchar()!='\n');
		switch(ret){
		case 1:
			admin_add(connfd,msg,len);
			break;
		case 2:
			admin_del(connfd,msg,len);
			break;
		case 3:
			view_msg(connfd,msg,len);
			break;
		case 4:
			change_msg_admin(connfd,msg,len);
			break;
		case 5:
			view_log(connfd,msg,len);
			break;
		case 6:
			quit_login(connfd,msg,len);
			return -1;
		default:
			printf("Input error!(press enter to continue)\n");
			fflush(stdout);
			while(getchar() != '\n');
			break;
		}
	}
	return 0;
}
/*管理员界面*/
void do_admin_view(void)
{
	system("clear");
	printf("\n");
	printf(" ┏━┓━━━━━━━━━━━━━━━━━━━━━┓━┓\n");
	printf(" ┃ ┃     管理员模式      ┃ ┃\n");
	printf(" ┃ ┗━━━━━━━━━━━━━━━━━━━━━┛ ┃\n");
	printf(" ┃ ┃[1]添加用户          ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┃ ┃[2]删除用户          ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┃ ┃[3]查看信息          ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┃ ┃[4]修改信息          ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┃ ┃[5]考勤记录          ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┃ ┃[6]退出登录          ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┗━┗━━━━━━━━━━━━━━━━━━━━━┛━┛\n");
}
/*输入信息*/
int input_msg(msg_cli* msg)
{
	printf("请输入密码　: ");
	fflush(stdout);
	fgets(msg->passwd,sizeof(msg->passwd),stdin);
	if(strlen(msg->passwd) < sizeof(msg->passwd))
		msg->passwd[strlen(msg->passwd)-1] = '\0';

	printf("******以下是员工信息******\n");
	printf("请输入工号: ");
	fflush(stdout);
	scanf("%d",&(msg->staff.id));
	while(getchar() != '\n');

	printf("请输入姓名: ");
	fflush(stdout);
	fgets(msg->staff.name,sizeof(msg->staff.name),stdin);
	//防止出现刚好的情况
	if(strlen(msg->staff.name) < sizeof(msg->staff.name))
		msg->staff.name[strlen(msg->staff.name)-1] = '\0';

	while(1){
		printf("请输入生日: ");
		fflush(stdout);
		fgets(msg->staff.birthday,sizeof(msg->staff.birthday),stdin);
		/*其实这里也可以用输入的是整数的值来判断*/
		//如果倒数第二位是'\n'则输入正确
		if(msg->staff.birthday[8] == '\n'){
			//计算一下年月日输入是否正确
			int year=0,month=0,day=0,i=0,j=0;
			char temp[5] = "";
			//年
			for(i=0; i<4; i++)
				temp[j++] = msg->staff.birthday[i];
			year = atoi(temp);
			bzero(temp,sizeof(temp));
			j = 0;
			//月
			for(i=4; i<6; i++)
				temp[j++] = msg->staff.birthday[i];
			month = atoi(temp);
			bzero(temp,sizeof(temp));
			j = 0;
			//日
			for(i=6; i<8; i++)
				temp[j++] = msg->staff.birthday[i];
			day = atoi(temp);
			if((year>1900) && (year<2022) && (month>0) && (month<13)\
					&& (day>0) && (day<32)){
				msg->staff.birthday[8] = '\0';
				break;
			}
		}else
			printf("格式错误(例子: 20210101)\n");
	}

	printf("请输入等级: ");
	fflush(stdout);
	scanf("%d",&(msg->staff.level));
	while(getchar() != '\n');

	printf("请输入部门: ");
	fflush(stdout);
	fgets(msg->staff.department,sizeof(msg->staff.department),stdin);
	if(strlen(msg->staff.department) < sizeof(msg->staff.department))
		msg->staff.department[strlen(msg->staff.department)-1] = '\0';

	printf("请输入职位: ");
	fflush(stdout);
	fgets(msg->staff.post,sizeof(msg->staff.post),stdin);
	if(strlen(msg->staff.post) < sizeof(msg->staff.post))
		msg->staff.post[strlen(msg->staff.post)-1] = '\0';

	printf("请输入薪水: ");
	fflush(stdout);
	scanf("%d",&(msg->staff.salary));
	while(getchar() != '\n');

	while(1){
		printf("请输入电话: ");
		fflush(stdout);
		fgets(msg->staff.telephone,sizeof(msg->staff.telephone),stdin);
		//如果倒数第二位是'\n'则输入正确
		if(msg->staff.telephone[11] == '\n'){
			msg->staff.telephone[11] = '\0';
			break;
		}else
			printf("输入格式错误!\n");
	}

	printf("请输入住址: ");
	fflush(stdout);
	fgets(msg->staff.address,sizeof(msg->staff.address),stdin);
	if(strlen(msg->staff.address) < sizeof(msg->staff.address))
		msg->staff.address[strlen(msg->staff.address)-1] = '\0';
}
/*管理员添加用户*/
int admin_add(int connfd,msg_cli* msg,int len)
{
	char username[20] = "";
	while(1){
		printf("请输入用户名: ");
		fflush(stdout);
		fgets(username,sizeof(username),stdin);
		if(strlen(username) < sizeof(username))
			username[strlen(username)-1] = '\0';
		//这里应该先判断一下用户名是否被占用
		msg->type = EXIST;
		strcpy(msg->data,username);
		cli_send(connfd,msg,len);
		cli_recv(connfd,msg,len);
		if(strcmp(msg->data,"NoExist") == 0){
			//用户名不重复,将username赋给data
			strcpy(msg->data,username);
			break;
		}
		else
			printf("用户名已存在!\n");
	}
	//将类型置为adduser
	msg->type = ADDUSER;

	input_msg(msg);
	if(cli_send(connfd,msg,len) == -1){
		return -1;
	}
	if(cli_recv(connfd,msg,len) == -1){
		return -1;
	}
	//判断服务器的反馈
	if(strcmp(msg->data,"OK") == 0){
		printf("\n写入成功!\n");
	}else{
		printf("\n写入失败,用户名重复!\n");
		return -1;
	}
	//printf("type=%d username=%s password=%s data=%s\nid=%d\nname=%s\nbirthday=%s\nlevel=%d\ndepartment=%s\npost=%s\nsalary=%d\ntelephone=%s\naddress=%s\n",\
	msg->type,msg->username,msg->passwd,msg->data,\
		msg->staff.id,msg->staff.name,msg->staff.birthday,msg->staff.level,msg->staff.department,\
		msg->staff.post,msg->staff.salary,msg->staff.telephone,msg->staff.address);
	printf("按Enter键继续");
	fflush(stdout);
	while(getchar() != '\n');
	return 0;
}
/*查看所有用户名*/
int admin_alluser(int connfd,msg_cli* msg,int len){
	msg->type = ALLUSER;
	cli_send(connfd,msg,len);
	printf("用户名:员工姓名\n");
	while(cli_recv(connfd,msg,len) != -1){
		if(strcmp(msg->data,"FindEnd") != 0){
			printf("%s\n",msg->data);
			fflush(stdout);
		}else{
			printf("**************************\n");
			break;
		}
	}
	return 0;
}
/*管理员删除用户*/
int admin_del(int connfd,msg_cli* msg,int len)
{
	char username[20] = "";
	char ch;
	//先打印出所有的用户名
	admin_alluser(connfd,msg,len);
	printf("请输入要删除的员工的用户名(#退出)-> ");
	fflush(stdout);
	fgets(username,sizeof(username),stdin);
	username[strlen(username)-1] = '\0';
	if(strcmp(username,"#") == 0);
	else if(strcmp(username,"admin") == 0){
		printf("不得删除当前用户账号!\n");
		printf("按Enter键继续");
		fflush(stdout);
		while(getchar() != '\n');
	}else{
		//确认一遍是否删除
		fprintf(stdout,"确认删除[%s]吗(y/n)?",username);
		fflush(stdout);
		ch = fgetc(stdin);
		while(getchar() != '\n');
		//只要y或Y就确认删除,n就不必判断了
		if(ch == 'y' || ch == 'Y'){
			//将类型置为删除用户
			msg->type = DELUSER;
			strcpy(msg->data,username);
			cli_send(connfd,msg,len);
			cli_recv(connfd,msg,len);
			//如果接收到OK表示删除成功
			if(strcmp(msg->data,"OK") == 0){
				printf("删除成功!\n");
			}else if(strcmp(msg->data,"username error") == 0){
				printf("用户名输入错误!\n");
			}
			printf("按Enter键继续");
			fflush(stdout);
			while(getchar() != '\n');
		}
	}
	return 0;
}
/*查看信息*/
int view_msg(int connfd,msg_cli* msg,int len)
{
	char buf[11][50] = {};
	int i = 0;
	//管理员模式
	if(strcmp(msg->username,"admin") == 0){
		char username[20] = "";
		//先查看所有用户
		admin_alluser(connfd,msg,len);

		printf("输入要查看的用户的用户名(#退出)-> ");
		fflush(stdout);
		fgets(username,sizeof(username),stdin);
		username[strlen(username)-1] = '\0';
		if(strcmp(username,"#") == 0)
			return -1;
		//将username给到data发送
		msg->type = VIEWMSG;
		strcpy(msg->data,username);
	}else{ //其他用户模式
		msg->type = VIEWMSG;
		strcpy(msg->data,msg->username);
	}
	//发送
	cli_send(connfd,msg,len);
	while(cli_recv(connfd,msg,len) != -1){
		//先排除用户名输入错误的情况
		if(strcmp(msg->data,"NoFind") == 0){
			printf("用户名输入错误!\n");
			printf("按Enter键继续");
			fflush(stdout);
			while(getchar() != '\n');
			return -1;
		}
		if(strcmp(msg->data,"end") != 0){
			strcpy(buf[i++],msg->data);
		}else break;
	}
	printf("用户名:%s\n密码  :%s\n工号  :%s\n姓名  :%s\n生日  :%s\n等级  :%s\n部门  :%s\n职位  :%s\n薪水  :%s\n电话  :%s\n住址  :%s\n\n",\
			buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10]);
	printf("按Enter键继续");
	fflush(stdout);
	while(getchar() != '\n');

	return 0;
}
/*管理员修改信息*/
int change_msg_admin(int connfd,msg_cli* msg,int len)
{
	char username[20] = "";
	int ret;
	//先查看所有用户
	admin_alluser(connfd,msg,len);

	while(1){
		printf("输入要修改的用户名(#退出)-> ");
		fflush(stdout);
		fgets(username,sizeof(username),stdin);
		username[strlen(username)-1] = '\0';
		//如果是#退出
		if(strcmp(username,"#") == 0){
			return -1;
		}
		//不得修改admin账号信息
		if(strcmp(username,msg->username) == 0){
			printf("\n不得修改admin账号信息(按Enter键继续)");
			fflush(stdout);
			while(getchar() != '\n');
			return -1;
		}
		//查看用户名是否存在
		msg->type = EXIST;
		strcpy(msg->data,username);
		cli_send(connfd,msg,len);
		cli_recv(connfd,msg,len);
		if(strcmp(msg->data,"exist") == 0){
			//用户名不存在,将username赋给data
			strcpy(msg->data,username);
			break;
		}
		else
			printf("用户名不存在!\n");
	}
	printf("\n**********以下为修改信息************\n\n");
	input_msg(msg);
	
	msg->type = CHANMSG;
	cli_send(connfd,msg,len);
	cli_recv(connfd,msg,len);

	if(strcmp(msg->data,"OK") == 0)
		printf("修改成功!\n");
	else
		printf("修改失败!\n");
	printf("输入Enter键继续");
	fflush(stdout);
	while(getchar() != '\n');
	return 0;
}
/*查看考勤记录*/
int view_log(int connfd,msg_cli* msg,int len)
{
	//管理员可查看全部,其他用户只能查看自己的
	if(strcmp(msg->username,"admin") == 0){
		//先查看所有用户
		admin_alluser(connfd,msg,len);
		//将用户名填到data中
		printf("请输入要查看的用户名-> ");
		fflush(stdout);
		fgets(msg->data,sizeof(msg->data),stdin);
		msg->data[strlen(msg->data)-1] = '\0';
	}else{ //普通用户
		//将用户名填到data中
		strcpy(msg->data,msg->username);
	}
	//将类型置为LOG查看考勤
	msg->type = LOG;
	cli_send(connfd,msg,len);
	//先接收上班打卡的,循环接收直到收到end
	printf("\n上班打卡\n");
	while(cli_recv(connfd,msg,len) != -1){
		if(strcmp(msg->data,"NoFind") == 0){
			break;
		}
		if(strcmp(msg->data,"end") == 0)
			break;
		else
			printf("%s",msg->data);
	}
	//再接收下班打卡的,循环接收直到收到end
	printf("\n下班打卡\n");
	while(cli_recv(connfd,msg,len) != -1){
		if(strcmp(msg->data,"NoFind") == 0){
			break;
		}
		if(strcmp(msg->data,"end") == 0)
			break;
		else
			printf("%s",msg->data);
	}
	//终端暂停等待
	printf("\n按Enter键继续");
	fflush(stdout);
	while(getchar() != '\n');
	return 0;
}
/*退出登录*/
int quit_login(int connfd,msg_cli* msg,int len)
{
	msg->type = QUIT;
	cli_send(connfd,msg,len);
	return 0;
}
/*其他用户操作*/
int do_other(int connfd,msg_cli* msg,int len)
{
	int ret;
	while(1){
		do_other_view();
		//输入选择
		printf("Please input your choose -> ");
		fflush(stdout);
		scanf("%d",&ret);
		while(getchar()!='\n');
		switch(ret){
		case 1:
			view_msg(connfd,msg,len);
			break;
		case 2:
			change_msg_other(connfd,msg,len);
			break;
		case 3:
			other_attendance(connfd,msg,len);
			break;
		case 4:
			view_log(connfd,msg,len);
			break;
		case 5:
			quit_login(connfd,msg,len);
			return -1;
		default:
			printf("Input error!(press enter to continue)\n");
			fflush(stdout);
			while(getchar() != '\n');
			break;
		}
	}
	return 0;
}
/*普通用户界面*/
void do_other_view(void)
{
	system("clear");
	printf("\n");
	printf(" ┏━┓━━━━━━━━━━━━━━━━━━━━━┓━┓\n");
	printf(" ┃ ┃       普通用户      ┃ ┃\n");
	printf(" ┃ ┗━━━━━━━━━━━━━━━━━━━━━┛ ┃\n");
	printf(" ┃ ┃[1]查看信息          ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┃ ┃[2]修改信息          ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┃ ┃[3]上下班打卡        ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┃ ┃[4]查看考勤          ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┃ ┃[5]退出登录          ┃ ┃\n");
	printf(" ┃ ┃                     ┃ ┃\n");
	printf(" ┗━┗━━━━━━━━━━━━━━━━━━━━━┛━┛\n");
}
/*普通用户修改信息*/
int change_msg_other(int connfd,msg_cli* msg,int len)
{
	msg->type = CHANMSG;
	printf("\n**********以下为修改信息************\n\n");
	printf("请输入密码: ");
	fflush(stdout);
	fgets(msg->passwd,sizeof(msg->passwd),stdin);
	if(strlen(msg->passwd) < sizeof(msg->passwd))
		msg->passwd[strlen(msg->passwd)-1] = '\0';

	while(1){
		printf("请输入生日: ");
		fflush(stdout);
		fgets(msg->staff.birthday,sizeof(msg->staff.birthday),stdin);
		/*其实这里也可以用输入的是整数的值来判断*/
		//如果倒数第二位是'\n'则输入正确
		if(msg->staff.birthday[8] == '\n'){
			//计算一下年月日输入是否正确
			int year=0,month=0,day=0,i=0,j=0;
			char temp[5] = "";
			//年
			for(i=0; i<4; i++)
				temp[j++] = msg->staff.birthday[i];
			year = atoi(temp);
			bzero(temp,sizeof(temp));
			j = 0;
			//月
			for(i=4; i<6; i++)
				temp[j++] = msg->staff.birthday[i];
			month = atoi(temp);
			bzero(temp,sizeof(temp));
			j = 0;
			//日
			for(i=6; i<8; i++)
				temp[j++] = msg->staff.birthday[i];
			day = atoi(temp);
			if((year>1900) && (year<2022) && (month>0) && (month<13)\
					&& (day>0) && (day<32)){
				msg->staff.birthday[8] = '\0';
				break;
			}
		}else
			printf("格式错误(例子: 20210101)\n");
	}

	while(1){
		printf("请输入电话: ");
		fflush(stdout);
		fgets(msg->staff.telephone,sizeof(msg->staff.telephone),stdin);
		//如果倒数第二位是'\n'则输入正确
		if(msg->staff.telephone[11] == '\n'){
			msg->staff.telephone[11] = '\0';
			break;
		}else
			printf("输入格式错误!\n");
	}

	printf("请输入住址: ");
	fflush(stdout);
	fgets(msg->staff.address,sizeof(msg->staff.address),stdin);
	if(strlen(msg->staff.address) < sizeof(msg->staff.address))
		msg->staff.address[strlen(msg->staff.address)-1] = '\0';
	
	cli_send(connfd,msg,len);
	cli_recv(connfd,msg,len);

	if(strcmp(msg->data,"OK") == 0)
		printf("修改成功!\n");
	else
		printf("修改失败!\n");
	printf("输入Enter键继续");
	fflush(stdout);
	while(getchar() != '\n');
	return 0;

}
/*普通用户打卡*/
int other_attendance(int connfd,msg_cli* msg,int len)
{
	msg->type = ADDLOG;
	int ret;
	printf("*************************\n");
	printf("*[1]上班打卡 [2]下班打卡*\n");
	printf("*************************\n");
	
	printf("请输入你的选择-> ");
	fflush(stdout);
	scanf("%d",&ret);
	while(getchar() != '\n');

	switch(ret){
		case 1:
			strcpy(msg->data,"login");
			break;
		case 2:
			strcpy(msg->data,"logout");
			break;
		default:
			printf("输入错误(按Enter键继续)");
			fflush(stdout);
			while(getchar() != '\n');
			return -1;
	}

	//发送给服务器后接收返回的data表示打卡与否
	cli_send(connfd,msg,len);
	cli_recv(connfd,msg,len);
	if(strcmp(msg->data,"OK") == 0)
		printf("打卡成功!(按Enter键继续)");
	else
		printf("打卡失败!(按Enter键继续)");
	fflush(stdout);
	while(getchar() != '\n');
	return 0;
}
//发送给服务器
int cli_send(int connfd,msg_cli* msg,int len)
{
	//printf("send:type:%2d username:%s passwd:%s data:%s\n",msg->type,msg->username,\
	msg->passwd,msg->data);
	int ret = send(connfd,(const void*)msg,len,0);
	if(ret <= 0){
		if(ret == 0){ //服务器关闭的情况
			fprintf(stderr,"Server close\n");
			return -1;
		}
		perror("send");
		return -1;//发生错误
	}
	return 0;
}
//接收服务器消息
int cli_recv(int connfd,msg_cli* msg,int len)
{
	//接收之前清空一下
	bzero(msg,len);
	int ret = recv(connfd,(void*)msg,len,0);
	if(ret <= 0){
		if(0 == ret){ //服务器关闭的情况
			fprintf(stderr,"Server close\n");
			return -1;
		}
		perror("recv");
		return -1;//发生错误
	}
	//printf("recv:type:%2d username:%s passwd:%s data:%s\n",msg->type,msg->username,\
	msg->passwd,msg->data);
	return 0;
}
