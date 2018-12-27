/*************************************************************************
    > File Name: server.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Wed 26 Dec 2018 08:57:19 PM CST
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
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t cli_addr_len;
	int sockfd;
	char buf[BUFSIZ];
	char str[INET_ADDRSTRLEN];
	int i, n;

	/* udp 报式数据流 */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	printf("Accepting connections ...\n");
	while(1){
		cli_addr_len = sizeof(cli_addr);

		/* 接收数据报的函数 相当于TCP中的 accept 和 read 函数的结合 */
		n = recvfrom(sockfd, buf, BUFSIZ, 0, (struct sockaddr *)&cli_addr, &cli_addr_len);
		if(n == -1){
			perror("recvfrom error");
		}

		printf("received from %s at Port %d\n",
				inet_ntop(AF_INET, &cli_addr.sin_addr, str, sizeof(str)),
				ntohs(cli_addr.sin_port));

		for(i = 0;i < n;i++)
			buf[i] = toupper(buf[i]);

		/* 相当于TCP中的 write函数 */
		n = sendto(sockfd, buf, n, 0, (struct sockaddr *)&cli_addr, sizeof(cli_addr));
		if(n == -1)
			perror("sendto error");
	}

	close(sockfd);

	return 0;
}

