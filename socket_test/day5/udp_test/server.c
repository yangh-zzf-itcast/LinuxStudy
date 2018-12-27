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
#include<pthread.h>
#include<sys/socket.h>
#include<errno.h>
#include<ctype.h>
#include<arpa/inet.h>

#define SERV_PORT 9528

void sys_err(const char* str){
	perror(str);
	exit(1);
}

int main(void)
{
	int rett;
	int sockfd;  
	char buf[BUFSIZ], client_IP[1025];
	struct sockaddr_in serv_addr;
	struct sockaddr_in client_addr;
	socklen_t client_addr_len;

	//第一步、创建udp服务器socket套接字,报式协议 SOCK_DGRAM
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	//为socket地址结构赋值
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//第二步、为套接字绑定IP、port
	bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	client_addr_len = sizeof(client_addr);

	while(1){
		//第三步、与客户端连接成功，开始读取数据, rett为读到的字节数
		rett = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_addr_len);


		printf("client ip:%s  port:%d\n", 
				inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_IP, sizeof(client_IP)), 
				ntohs(client_addr.sin_port));

		int i;
		for(i = 0;i < rett;++i){
			buf[i] = toupper(buf[i]);
		}

		sendto(sockfd, buf, rett, 0, (struct sockaddr *)&client_addr, client_addr_len);
	
		write(STDOUT_FILENO, buf, rett);
	}

	close(sockfd);

	return 0;
}
