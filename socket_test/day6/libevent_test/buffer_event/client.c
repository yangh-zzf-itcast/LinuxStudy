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

	char *p = "客户端成功接受终端数据，并发送到服务器上！";
	bufferevent_write(bev, p, strlen(p) + 1);
	//调用bufferevent_write后，写缓存中有了数据，会调用写回调函数write_cb
	
	sleep(1);
}

void write_cb(struct bufferevent *bev, void *arg)
{
	printf("客户端接收到服务器数据...\n");
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


//客户从终端读取数据，然后写给服务器
void read_terminal(evutil_socket_t fd, short what, void *arg)
{
	//读数据
	char buf[1024] = {0};
	int len = read(fd, buf, sizeof(buf));

	//向服务器写数据
	struct bufferevent *bev = (struct bufferevent *)arg;
	bufferevent_write(bev, buf, len+1);
}

int main(int argc, char *argv[])
{

	//创建一个base结构体底座
	struct event_base *base;
	base = event_base_new();

	struct sockaddr_in serv;

	bzero(&serv, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr.s_addr);

	int fd = socket(AF_INET, SOCK_STREAM, 0);

	//创建bufferevent 事件，将fd放入
	struct bufferevent *bev = NULL;
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	
	//链接服务器
	bufferevent_socket_connect(bev, (struct sockaddr *)&serv, sizeof(serv));

	//设置回调
	//bufferevent_setcb(bev, read_cb, write_cb, event_cb, NULL);
	bufferevent_setcb(bev, read_cb, write_cb, NULL, NULL);

	//设置读事件使能
	bufferevent_enable(bev, EV_READ);

	//创建事件，对 标准输入 文件描述符作监听
	struct event *ev = event_new(base, STDIN_FILENO, EV_READ | EV_PERSIST,
								read_terminal, bev);

	event_add(ev, NULL);

	event_base_dispatch(base);

	event_free(ev);
	
	event_base_free(base);

	return 0;
}
