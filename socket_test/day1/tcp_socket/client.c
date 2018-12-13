/*************************************************************************
    > File Name: client.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Thu 06 Dec 2018 07:47:49 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<ctype.h>

#define SERV_PORT 9528   //严格对应要连接的服务端的地址

void sys_err(const char *str){
	perror(str);
	exit(1);
}

int main(void)
{
	int ret, count = 10;
	int cfd =0;
	char buf[BUFSIZ];
	socklen_t len;
	struct sockaddr_in serv_addr;

	//为要链接的服务端socket地址赋值
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr.s_addr);
	//serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//第一步，创建客户端的套接字
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if(cfd == -1){
		sys_err("socket error");
	}

	len = sizeof(serv_addr);
	//连接服务端
	ret = connect(cfd, (struct sockaddr*)&serv_addr, len);
	if(ret == -1){
		sys_err("connect error");
	}
	while(--count){
		//向服务器写入数据
		write(cfd, "hello world\n", 12);
		sleep(1);
		ret = read(cfd, buf, sizeof(buf));
		write(STDOUT_FILENO, buf, ret);
	}

	close(cfd);

	return 0;
}
