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
#define SRV_PORT 9529


int main(int argc, char *argv[])
{
	int i, j, n, maxi;
	int nready, client[1024];		//FD_SETSIZ 宏定义，默认1024
	int listenfd, connfd, sockfd;
	int opt = 1, maxfd = 0;   //maxfd 标记文件描述符最大的那个

	char buf[BUFSIZ], str[INET_ADDRSTRLEN];		//宏定义，16
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

	//最大文件描述符
	maxfd = listenfd;

	maxi = -1;		//将来用作client[]的下标，初始值指向0个元素之前的下标位置
	for(i = 0;i < 1024;i++){	//-1用来初始化 client[], client[]负责存客户的读事件文件描述符集
		client[i] = -1;	    	//不存listenfd
	}

	//对文件描述符集合进行操作
	FD_ZERO(&allset);	//清空 监听集合操作
	FD_SET(listenfd, &allset);	//将lfd添加到allset操作

	while(1){
		rset = allset;		//每一次循环时都重新设置select监控信号集
		nready = select(maxfd + 1, &rset, NULL, NULL, NULL);  //nready是满足读事件的fd个数
		if(nready < 0){
			perr_exit("select error");
		}
		//连接客户端读事件的判断
		if(FD_ISSET(listenfd, &rset)){		//判断listenfd是否满足监听 读事件，满足说明收到新的客户端连接请求
			clt_addr_len = sizeof(clt_addr);

			//lfd满足 读事件时服务器才会调用 accept 函数，因此不会造成阻塞
			connfd = Accept(listenfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
			printf("received from %s at port %d \n",
					inet_ntop(AF_INET, &clt_addr.sin_addr, str, sizeof(str)),
					ntohs(clt_addr.sin_port));

			for(i = 0;i < 1024;i++){		//找到client[]还没被使用的位置
				if(client[i] < 0){				//将lfd监听到的connfd保存到client[]里
					client[i] = connfd;
					break;
				}
			}

			//达到一开始设置的数组大小，client监听上限
			if(i == 1024){
				fputs("too much clients\n", stderr);
				exit(1);
			}

			FD_SET(connfd, &allset);	//向监听文件描述符集合allset添加接受链接的connfd，监听数据的读事件

			if(maxfd < connfd)			//修改最大 文件描述符
				maxfd = connfd;

			if(i > maxi)
				maxi = i;				//maxi保存的总是client[]最后一个非-1元素的下标

			if(0 == --nready)	//说明只有listenfd有事件，数量是1，没有其他通信事件，后续for循环不需要操作
				continue;		//继续监听
		}

		//数据读事件的判断
		//当返回满足事件的文件描述符不止有listenfd时，其他的cfd需要进行读写操作
		//直接检测client[]保存的数组个数就行
		for(i = 0;i <= maxi;i++){
			sockfd = client[i];
			if(sockfd < 0)
				continue;
			if(FD_ISSET(sockfd, &rset)){			//检测满足读事件的文件描述符是哪一个
				//从客户端读到0，表示客户端已经关闭连接，服务器端也应关闭对应连接
				if((n = Read(sockfd, buf, sizeof(buf))) == 0){
					close(sockfd);				//关闭对应的connfd
					FD_CLR(sockfd, &allset);	//解除select对此文件描述符的监控，移除集合
					client[i] = -1;				//将对应位置的cilent[i] 置为初始值
					printf("stop connect from %s at port %d\n",
							inet_ntop(AF_INET, &clt_addr.sin_addr, str, sizeof(str)),
							ntohs(clt_addr.sin_port));
				}else if(n > 0){
					for(j = 0;j < n;j++)
						buf[j] = toupper(buf[j]);
					Write(sockfd, buf, n);
					Write(STDOUT_FILENO, buf, n);
				}else if(n == -1){
					perr_exit("read error");
				}
				if(--nready == 0)		//监听到的个数是1，并且不是lfd
					break;		//跳出for循环，但还在while循环中, 说明client[]操作完了
			}
		}
	}

	close(listenfd);

	return 0;
}

