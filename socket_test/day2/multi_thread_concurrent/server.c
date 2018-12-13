/*************************************************************************
    > File Name: server.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Thu 13 Dec 2018 07:41:35 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<unistd.h>
#include<fcntl.h>
#include<pthread.h>
#include<errno.h>
#include<sys/socket.h>
#include<signal.h>
#include<arpa/inet.h>
#include<ctype.h>

#include"wrap.h"
#define MAXLINE 8192
#define SRV_PORT 9000

struct s_info{		//定义一个结构体，封装socket地址结构和socket文件描述符cfd
	struct sockaddr_in cliaddr;
	int connfd;
};

//子线程负责的工作
void *do_work(void *arg)
{
	int n, i;
	struct s_info *ts = (struct s_info*)arg;
	char buf[MAXLINE];
	char str[INET_ADDRSTRLEN];		//#define = 16

	while(1){
		n = Read(ts->connfd, buf, MAXLINE);		//读取客户端数据

		if(n == 0){
			printf("the client %d closed....\n", ts->connfd);
			break;		//跳出循环，关闭cfd
		}

		printf("received from %s at PORT %d \n",
				inet_ntop(AF_INET, &(*ts).cliaddr.sin_addr, str, sizeof(str)), ntohs((*ts).cliaddr.sin_port));    //打印客户端的信息 IP port

		for(i = 0;i < n;i++){
			buf[i] = toupper(buf[i]);
		}

		Write(ts->connfd, buf, n);
		Write(STDOUT_FILENO, buf, n);
	}

	close(ts->connfd);
	//return (void *)0;
	pthread_exit(0);
}

int main(void)
{
	
	int listenfd, connfd;
	int ret, i = 0;
	pthread_t tid;
	struct s_info ts[256];		//创建结构体数组

	struct sockaddr_in servaddr, cliaddr;
	socklen_t cliaddr_len;
	cliaddr_len = sizeof(cliaddr);

	//将地址结构清零操作
	//memset(&srv_addr, 0, sizeof(srv_addr)); 
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family      = AF_INET;
	servaddr.sin_port		 = htons(SRV_PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);

	Bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	Listen(listenfd, 128);

	printf("Accepting client connect.....\n");

	while(1){
		connfd = Accept(listenfd, (struct sockaddr*)&cliaddr, &cliaddr_len);
		//监听到一个客户端的链接请求，就给结构体赋值
		ts[i].connfd = connfd;
		ts[i].cliaddr = cliaddr;
		
		//创建子线程
		pthread_create(&tid, NULL, do_work, (void*)&ts[i]);
		//父线程与子线程进行分离，防止僵尸进程产生
		pthread_detach(tid);
		i++;
	}

	return 0;
}

