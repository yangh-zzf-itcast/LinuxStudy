/*************************************************************************
    > File Name: server.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Sun 16 Dec 2018 07:51:35 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<sys/socket.h>
#include<signal.h>
#include<arpa/inet.h>
#include<ctype.h>

#include<sys/epoll.h>
#include"wrap.h"

#define MAXLINE 80
#define OPEN_MAX 1024
#define SRV_PORT 8000


int main(int argc, char *argv[])
{
	int i, j, n, ret, cnt;
	int listenfd, connfd, sockfd;
	int opt = 1;   

	char buf[MAXLINE], str[INET_ADDRSTRLEN];		//宏定义，16
	socklen_t clt_addr_len;

	struct epoll_event tep, ep[1024];	//定义event 结构体变量 和 结构体数组
	struct sockaddr_in serv_addr, clt_addr;
	clt_addr_len = sizeof(clt_addr);
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family   	  = AF_INET;
	serv_addr.sin_port		  = htons(SRV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	//对端口地址进行复用
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	Bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	Listen(listenfd, 128);

	int epfd = epoll_create(1024);		//创建监听红黑树的根节点

	tep.events = EPOLLIN;
	tep.data.fd = listenfd;		//给event结构体变量赋值

	cnt = epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &tep);		//将lfd添加到监听红黑树上
	if(cnt < 0)
		perr_exit("epoll_wait error");

	while(1){
		ret	= epoll_wait(epfd, ep, 1024, 1);	//实施监听，并返回满足事件的结构体和结构体个数
		if(ret < 0){
			perr_exit("epoll_wait error");
		}
		for(i = 0;i < ret;i++){
			sockfd = ep[i].data.fd;
			//首先判断满足事件的节点是lfd 还是 connfd
			if(sockfd == listenfd){		//lfd满足读事件，说明有客户端请求链接
				connfd = Accept(listenfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
				//建立连接之后，将connfd 客户端文件描述符挂到监听红黑树epfd上
				tep.events = EPOLLIN;
				tep.data.fd = connfd;
				cnt = epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &tep);
				if(cnt < 0)
					perr_exit("epoll_ctl error");
			}else{		//connfd 们 满足读事件，有客户端传输数据到服务器
				n = Read(sockfd, buf, sizeof(buf));
				if(n == 0){		//说明对端已经关闭链接
					cnt = epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);		//将这个connfd从监听红黑树上摘下
					if(cnt < 0)
						perr_exit("epoll_ctl error");
					close(sockfd);
				}else if(n > 0){
					for(i = 0;i < n;i++){
						buf[i] = toupper(buf[i]);
					}
					Write(sockfd, buf, n);
				}else{
					perr_exit("read error");
					/*if(errno = ECONNREST){		//连接被重置，关闭文件描述符，并从树上移除
						close(sockfd);
						cnt = epoll_ctl(epfd, EPOLL_CTL_DEL, sockfd, NULL);
						if(cnt < 0)
							perr_exit("epoll_ctl error");
					}else{
						perr_exit("read error");
					}*/
				}
			}
		}
	}
	close(listenfd);

	return 0;
}

