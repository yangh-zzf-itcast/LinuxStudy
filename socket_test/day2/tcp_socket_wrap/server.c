/*************************************************************************
    > File Name: server.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Thu 06 Dec 2018 07:06:08 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<sys/socket.h>
#include<errno.h>
#include<ctype.h>
#include<arpa/inet.h>
#include"wrap.h"

#define SERV_PORT 9528

int main(void)
{
	int ret;
	int rett;
	int lfd = 0;  
	int cfd = 0;
	char buf[BUFSIZ], client_IP[1025];
	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;

	//第一步、创建服务器socket套接字
	lfd = Socket(AF_INET, SOCK_STREAM, 0);

	//为socket地址结构赋值
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//第二步、为套接字绑定IP、port
	Bind(lfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));


	//第三步、设置服务器同时监听的最大上限值
	Listen(lfd, 128);

	//第四步，阻塞接受客户端请求，接受成功生成一个新的socket，最先的继续监听
	client_addr_len = sizeof(client_addr);
	cfd = Accept(lfd, (struct sockaddr*)&client_addr, &client_addr_len);
	printf("client ip:%s  port:%d\n", inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)), ntohs(client_addr.sin_port));

	while(1){
		//与客户端连接成功，开始读取数据, rett为读到的字节数
		rett = Read(cfd, buf, sizeof(buf));
		if(rett == -1){
			perror("read error");
		}
		Write(STDOUT_FILENO, buf, rett);
		int i;
		for(i = 0;i < rett;++i){
			buf[i] = toupper(buf[i]);
		}

		//将大小写转换后，写回客户端
		Write(cfd, buf, rett);
	}

	close(lfd);
	close(cfd);

	return 0;
}
