/*************************************************************************
    > File Name: client.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Wed 26 Dec 2018 09:24:53 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<ctype.h>
#include<arpa/inet.h>

#define SERV_PORT 8000

int main(void)
{
	struct sockaddr_in servaddr;
	int sockfd, n;
	char buf[BUFSIZ];

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_port = htons(SERV_PORT);

	/* bind步骤 无效 ，不加则客户端 进行 隐式绑定*/
	//bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	while(fgets(buf, BUFSIZ, stdin) != NULL){
		/* sendto函数 相当于 原来TCP客户端中的 read + connect 函数结合 */
		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if(n == -1)
			perror("sendto error");

		n = recvfrom(sockfd, buf, BUFSIZ, 0, NULL, 0);	//后面写NULL，不需要关心对端的信息
		if(n == -1)
			perror("recvfrom error");

		write(STDOUT_FILENO,buf, n);
	}

	close(sockfd);

	return 0;
}

