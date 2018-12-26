/*************************************************************************
    > File Name: server.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Sat 22 Dec 2018 08:21:57 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/epoll.h>

#include<arpa/inet.h>
#include<netinet/in.h>

#define MAXLINE 10
#define SERV_PORT 9000

int main(void)
{
	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	int listenfd, connfd;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];
	int efd;

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	listen(listenfd, 20);

	printf("Accepting connections....\n");
	cliaddr_len = sizeof(cliaddr);
	connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);

	printf("Receive from %s at Port %d \n", 
			inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), 
			ntohs(cliaddr.sin_port));

	struct epoll_event event;
	struct epoll_event revents[10];
	int res, len;	

	efd = epoll_create(10);
	event.events = EPOLLIN | EPOLLET;
	//event.events = EPOLLIN;
	event.data.fd = connfd;
	epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event);	//树上只监听connfd，不监听listenfd

	while(1){
		res = epoll_wait(efd, revents, 10, -1);
		printf("res = %d\n", res);
		if(revents[0].data.fd == connfd){
			len = read(connfd, buf, MAXLINE/2);
			write(STDOUT_FILENO, buf, len);
		}
	}

	close(efd);
	close(listenfd);
	close(connfd);
	return 0;
}
