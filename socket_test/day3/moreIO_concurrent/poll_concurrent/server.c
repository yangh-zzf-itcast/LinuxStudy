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

#include<poll.h>
#include"wrap.h"

#define MAXLINE 80
#define OPEN_MAX 1024
#define SRV_PORT 8000


int main(int argc, char *argv[])
{
	int i, j, n, maxi;
	int nready;		//接收poll返回值， 记录满足监听事件的fd个数
	int listenfd, connfd, sockfd;
	int opt = 1;   

	char buf[MAXLINE], str[INET_ADDRSTRLEN];		//宏定义，16
	socklen_t clt_addr_len;

	struct pollfd client[OPEN_MAX];		//定义poll结构体
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

	//对poll数组第一位存入listenfd，并让listenfd监听读事件
	client[0].fd = listenfd;
	client[0].events = POLLIN;

	//将client数组中剩余元素用-1初始化
	for(i = 1;i < OPEN_MAX;i++){
		client[i].fd = -1;
	}

	maxi = 0;		//client[]数组中有效元素的最大下标

	while(1){
		nready = poll(client, maxi + 1, -1);  //阻塞监听是否有客户端的连接请求
		if(nready < 0){
			perr_exit("poll error");
		}
		//连接客户端读事件的判断
		if(client[0].revents & POLLIN){		//判断listenfd是否满足监听 读事件
			clt_addr_len = sizeof(clt_addr);

			//lfd满足 读事件时服务器才会调用 accept 函数，因此不会造成阻塞
			connfd = Accept(listenfd, (struct sockaddr*)&clt_addr, &clt_addr_len);
			printf("received from %s at port %d \n",
					inet_ntop(AF_INET, &clt_addr.sin_addr, str, sizeof(str)),
					ntohs(clt_addr.sin_port));

			for(i = 0;i < OPEN_MAX;i++){		//找到client[]数组中没被使用的位子
				if(client[i].fd < 0){				//将lfd监听到的connfd保存到client[]里
					client[i].fd = connfd;
					break;
				}
			}

			//达到一开始设置的数组大小，client监听上限
			if(i == OPEN_MAX){
				fputs("too much clients\n", stderr);
				exit(1);
			}

			client[i].events = POLLIN;		//设置刚lfd监听到的connfd，监控读事件

			if(i > maxi)
				maxi = i;				//maxi保存的总是client[]最后一个非-1元素的下标

			if(0 == --nready)	//说明只有listenfd有事件，数量是1，没有其他通信事件，后续for循环不需要操作
				continue;		//继续监听
		}

		//数据读事件的判断
		//前面的if没满足，listenfd没有满足的，判断有没有connfd满足事件
		//直接检测client[]保存的数组个数就行
		for(i = 0;i <= maxi;i++){
			sockfd = client[i].fd;
			if(sockfd < 0)
				continue;
			if(client[i].revents & POLLIN){			//检测满足读事件的文件描述符是哪一个
				//从客户端读到0，表示客户端已经关闭连接，服务器端也应关闭对应连接
				if((n = Read(sockfd, buf, sizeof(buf))) < 0){
					if(errno == ECONNRESET){	//收到RST标志，重置
						printf("client[%d] from %s at port %d aborted connection\n",i ,
							   inet_ntop(AF_INET, &clt_addr.sin_addr, str, sizeof(str)),
							   ntohs(clt_addr.sin_port));
						close(sockfd);
						client[i].fd = -1;		//poll中不再监听该文件描述符，关闭置-1
					}else{
						perr_exit("read error");
					}
				}else if(n == 0){		//说明对端关闭链接
					printf("client[%d] from %s at port %d closed connection\n",i ,
						   inet_ntop(AF_INET, &clt_addr.sin_addr, str, sizeof(str)),
						   ntohs(clt_addr.sin_port));
					close(sockfd);
					client[i].fd = -1;
				}else{
					for(j = 0;j < n;j++)
						buf[j] = toupper(buf[j]);
					Write(sockfd, buf, n);
					Write(STDOUT_FILENO, buf, n);
				}
				if(--nready <= 0)
					break;
			}
		}
	}

	close(listenfd);

	return 0;
}

