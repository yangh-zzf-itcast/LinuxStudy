/*************************************************************************
    > File Name: epoll.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Sat 22 Dec 2018 07:35:36 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/epoll.h>
#include<error.h>

#define MAXLINE 10

int main(int argc, char *argv[])
{
	int efd, i;
	int pfd[2];		//管道的读端和写端，0是读，1是写
	pid_t pid;
	char buf[MAXLINE], ch = 'a';

	pipe(pfd);		//创建一根管道
	pid = fork();	//创建子进程

	if(pid == 0){	//子进程在负责管道的写，因此把读端关闭
		close(pfd[0]);
		while(1){
			for(i = 0;i < MAXLINE/2;i++){
				buf[i] = ch;
			}
			buf[i - 1] = '\n';
			ch++;

			for(;i < MAXLINE;i++){
				buf[i] = ch;
			}
			buf[i - 1] = '\n';
			ch++;
			write(pfd[1], buf, sizeof(buf));
			sleep(5);
		}
		close(pfd[1]);
	}else if(pid > 0){	//父进程负责管道的读端，将写关闭
		close(pfd[1]);
		
		struct epoll_event event;
		struct epoll_event revents[10];
		int res, len;

		efd = epoll_create(10);

		event.events = EPOLLIN | EPOLLET;	//ET边缘触发模式， 事件被监听到满足才进行触发 
		//event.events = EPOLLIN;		//LT水平触发模式，默认。有数据就进行触发
		event.data.fd = pfd[0];
		epoll_ctl(efd, EPOLL_CTL_ADD, pfd[0], &event);

		while(1){
			res = epoll_wait(efd, revents, 10, -1);
			printf("res = %d\n", res);
			if(revents[0].data.fd == pfd[0]){
				len = read(pfd[0], buf, MAXLINE/2);
				write(STDOUT_FILENO, buf, len);
			}
		}
		close(pfd[0]);
		close(efd);
	}else{
		perror("fork");
		exit(-1);
	}

	return 0;
}

