/*************************************************************************
    > File Name: myhttpd0.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Sun 06 Jan 2019 05:59:43 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <strings.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

#include "epoll_server.h"
#include "baseFun.h"

int main(int argc, char *argv[])
{
	//利用命令行参数获取 端口以及 server提供的 服务器根目录
	if(argc < 3){
		printf("./server port path\n");
	}

	//获取用户输入的端口
	int port = atoi(argv[1]);

	//改变进程的工作目录
	int ret = chdir(argv[2]);
	if(ret != 0)
	{
		sys_err("chdir error");
	}

	// 启动 epoll框架服务器监听
	epoll_run(port);

	return 0;
}

