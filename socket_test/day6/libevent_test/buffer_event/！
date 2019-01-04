/*************************************************************************
    > File Name: server.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Fri 04 Jan 2019 09:30:41 AM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <strings.h>
#include <sys/stat.h>

#include <event2/event.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

#define SERV_PORT 9876

void read_cb(struct bufferevent *bev, void *arg)
{
	char buf[1024] = {0};
	
	bufferevent_read(bev, buf, sizeof(buf));
	printf("client say: %s\n", buf);

	char *p = "服务器成功接收到您的数据！";
	bufferevent_write(bev, p, strlen(p) + 1);
	//调用bufferevent_write后，写缓存中有了数据，会调用写回调函数write_cb
	
	sleep(1);
}

void write_cb(struct bufferevent *bev, void *arg)
{
	printf("服务器，成功写数据给客户端，写缓冲区回调函数被调用...\n");
}

//bev是在cb_listener中创建的对象
void event_cb(struct bufferevent *bev, short events, void *arg)
{
	if(events & BEV_EVENT_EOF)
	{
		printf("connection closed\n");
	}else if(events & BEV_EVENT_ERROR){
		printf("some error happened\n");
	}

	bufferevent_free(bev);

	printf("bufferevent对象已被释放！\n");
}

void cb_listener(struct evconnlistener *listener,
		  evutil_socket_t  sock,  struct sockaddr *addr,  int len,  void *ptr)
{
	printf("connect new client[%d]\n", sock);

	//接受泛型指针为参数，将主函数中创建的 base 对象传进来，用于添加新事件到base上
	struct event_base *base = (struct event_base *)ptr;

	//添加新事件, 绑定套接字 fd
	struct bufferevent *bev;
	bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);

	//给bufferevent缓冲区设置回调
	//bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
	bufferevent_setcb(bev, read_cb, write_cb, NULL, NULL);

	//默认读缓冲disable，写缓冲enable，将读缓冲也启用
	bufferevent_enable(bev, EV_READ);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in serv;

	bzero(&serv, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(SERV_PORT);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);

	//创建一个base结构体底座
	struct event_base *base;
	base = event_base_new();

	//这个函数实现创建套接字、绑定、接受连接请求
	//bufferevent 的对象定义在监听回调函数中，监听到客户端建立了链接，再进行数据读写
	//将base作为回调函数参数传入，在回调cd_listener中会用到，如果不同的话传NULL
	//创建了一个 listener 监听器
	struct evconnlistener *listener;
	listener = evconnlistener_new_bind(base, cb_listener, (void *)base,
			LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, -1, 
			(struct sockaddr *)&serv, sizeof(serv));

	//设置循环监听
	event_base_dispatch(base);

	evconnlistener_free(listener);
	
	event_base_free(base);

	return 0;
}
