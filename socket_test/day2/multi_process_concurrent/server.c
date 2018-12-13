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
#include<pthread.h>
#include<errno.h>
#include<sys/socket.h>
#include<signal.h>
#include<arpa/inet.h>
#include<ctype.h>

#include"wrap.h"
#define SRV_PORT 9999

void catch_child(int signum)
{
	while((waitpid(0, NULL, WNOHANG)) > 0);
	return;
}

int main(int argc, char *argv[])
{
	
	int lfd, cfd;
	pid_t pid;
	int ret, i;
	char buf[BUFSIZ];

	struct sockaddr_in srv_addr, clt_addr;
	socklen_t clt_addr_len;
	clt_addr_len = sizeof(clt_addr);

	//将地址结构清零操作
	//memset(&srv_addr, 0, sizeof(srv_addr)); 
	bzero(&srv_addr, sizeof(srv_addr));

	srv_addr.sin_family      = AF_INET;
	srv_addr.sin_port		 = htons(SRV_PORT);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	lfd = Socket(AF_INET, SOCK_STREAM, 0);

	Bind(lfd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

	Listen(lfd, 128);

	while(1){
		cfd = Accept(lfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
		printf("cfd = %d\n", cfd);	
		//接收到一个客户端之后，创建一个子进程
		pid = fork();

		if(pid < 0){
			perr_exit("fork error");
		}else if(pid == 0){		//子进程关闭用于监听的lfd，用cfd进行通信，对数据进行操作
			close(lfd);
			break;
		}else{					//父进程关闭用于通信的cfd，用lfd继续监听是否有客户端链接
		    struct sigaction act;
			act.sa_handler = catch_child;
			sigemptyset(&act.sa_mask);
			act.sa_flags = 0;

			ret = sigaction(SIGCHLD, &act, NULL);		//
			if(ret != 0){
				perr_exit("sigaction error");
			}
			close(cfd);
			continue;
		}
	}

	if(pid == 0){
		while(1){
			ret = Read(cfd, buf, sizeof(buf));
			if(ret == 0){
				close(cfd);		//ret = 0 检测到服务器关闭，子进程关闭cfd
				exit(1);
			}

			for(i = 0;i < ret;i++){
				buf[i] = toupper(buf[i]);
			}

			Write(cfd, buf, ret);
			Write(STDOUT_FILENO, buf, ret);
			}
	}

	return 0;
}

