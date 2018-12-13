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
#include"wrap.h"

#define SERV_PORT 9528   //严格对应要连接的服务端的地址

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
	cfd = Socket(AF_INET, SOCK_STREAM, 0);

	len = sizeof(serv_addr);
	//连接服务端
	Connect(cfd, (struct sockaddr*)&serv_addr, len);

	while(--count){
		//向服务器写入数据
		Write(cfd, "hello world\n", 12);
		sleep(1);
		ret = Read(cfd, buf, sizeof(buf));
		Write(STDOUT_FILENO, buf, ret);
	}

	close(cfd);

	return 0;
}
