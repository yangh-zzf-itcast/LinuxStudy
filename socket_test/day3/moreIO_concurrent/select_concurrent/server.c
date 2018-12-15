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
#define SRV_PORT 6666


int main(int argc, char *argv[])
{
	int i, j, n;
	int nready;
	int listenfd, connfd;
	int opt = 1, maxfd = 0;   //maxfd 标记文件描述符最大的那个

	char buf[BUFSIZ];
	//定义文件描述符集合，rset是监听读事件的集合，allset是暂存所有描述符的集合，备份读集合
	fd_set rset, allset; 

	socklen_t clt_addr_len;
	struct sockaddr_in serv_addr, clt_addr;
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family   	  = AF_INET;
	serv_addr.sin_port		  = htons(SRV_PORT);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	//对端口地址进行复用
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	Bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	Listen(listenfd, 128);

	maxfd = listenfd;

	//对文件描述符集合进行操作
	FD_ZERO(&allset);	//清空操作
	FD_SET(listenfd, &allset);	//将lfd添加到allset操作

	while(1){
		rset = allset;		//每一次循环时都重新设置select监控信号集
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);  //nready是满足读事件的fd个数
		if(nready < 0){
			perr_exit("select error");
		}
		if(FD_ISSET(listenfd, &rset)){		//判断listenfd是否满足读事件，满足说明收到新的客户端连接请求
			clt_addr_len = sizeof(clt_addr);
			connfd = Accept(listenfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
			
			FD_SET(connfd, &allset);	//向监控文件描述符集合allset添加接受链接的connfd

			if(maxfd < connfd)
				maxfd = connfd;

			if(0 == --nready)	//说明只有listenfd有事件，没有其他通信事件，后续for循环不需要操作
				continue;		//继续监听
		}

		//当返回满足事件的文件描述符不止有listenfd时，其他的cfd需要进行读写操作
		for(i = listenfd + 1;i <= maxfd;i++){
			if(FD_ISSET(i, &rset)){		//检测满足读事件的文件描述符是哪一个
				//从客户端读到0，表示客户端已经关闭连接，服务器端也应关闭对应连接
				if((n = Read(i, buf, sizeof(buf))) == 0){
					close(i);
					FD_CLR(i, &allset);	//解除select对此文件描述符的监控
				}else if(n > 0){
					for(j = 0;j < n;j++)
						buf[j] = toupper(buf[j]);
					Write(i, buf, n);
					Write(STDOUT_FILENO, buf, n);
				}

			}
		}

	}

	close(listenfd);

	return 0;
}

