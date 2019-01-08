/*************************************************************************
    > File Name: main.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Mon 07 Jan 2019 09:38:49 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <strings.h>
#include <sys/stat.h>
#include <signal.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>

#include "lib_server.h"
#include "baseFun.h"

int main(int argc, char *argv[])
{
	if(argc < 3){
		printf("./event_http port path\n");
		return -1;
	}

	int port = atoi(argv[1]);

	if(chdir(argv[2]) < 0){
		printf("dir is not exist: %s\n", argv[2]);
		perror("chdir error:");
		return 1;
	}

	struct event_base *base;
	struct evconnlistener *listener;
	struct event *signal_event;

	struct sockaddr_in sin;
	bzero(&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	base = event_base_new();
	if(!base){
		fprintf(stderr, "Could not initialize libevent!\n");
		return 1;
	}

	//创建监听的套接字、绑定、监听、接受浏览器的连接请求
	//在listener_cb 回调函数中进行创建、绑定、监听和接受浏览器客户端
	listener = evconnlistener_new_bind(base, listener_cb, (void *)base,
				LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
				(struct sockaddr *)&sin, sizeof(sin));
	if(!listener){
		fprintf(stderr, "Could not create a listener!\n");
		return 1;
	}

	//创建信号事件，添加到底座base上，捕捉到信号之后并且处理
	//signal_cb 是信号捕捉回调函数
	signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);
	if(!signal_event || event_add(signal_event, NULL) < 0){
		fprintf(stderr, "Could not create/add a signal_event!\n");
		return 1;
	}

	//启动事件循环
	event_base_dispatch(base);

	//释放资源
	evconnlistener_free(listener);
	event_free(signal_event);
	event_base_free(base);

	printf("Done....\n");

	return 0;
}
