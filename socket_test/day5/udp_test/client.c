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
#include<sys/socket.h>
#include<arpa/inet.h>
#include<ctype.h>

#define SERV_PORT 9528   //严格对应要连接的服务端的地址
#define SERV_IP "127.0.0.1"

int main(void)
{
	int ret;
	int sockfd;
	char buf[BUFSIZ];
	//socklen_t len;
	struct sockaddr_in serv_addr;

	//为要链接的服务端socket地址赋值
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, SERV_IP, &serv_addr.sin_addr.s_addr);
	//serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//第一步，创建客户端的套接字
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);


	while(1){
		fgets(buf, sizeof(buf), stdin);
		sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

		ret = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, 0);
		
		write(STDOUT_FILENO, buf, ret);
	}

	close(sockfd);

	return 0;
}
