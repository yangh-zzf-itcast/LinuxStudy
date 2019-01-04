/*************************************************************************
    > File Name: read_fifo_event.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Wed 02 Jan 2019 07:04:56 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <event2/event.h>

void sys_err(const char *str)
{
	perror(str);
	exit(1);
}

void write_cb(evutil_socket_t fd, short what, void *arg)
{
	char buf[] = "hello libevent";
	int len = write(fd, buf, strlen(buf) + 1);
	printf("what = %s\n", what & EV_WRITE ? "write 满足": "write不满足");
	printf("write : %s,  len = %d\n", buf, len);
	sleep(1);
	return;
}

int main(int argc, char *argv[])
{

	//打开fifo的写端
	int fd = open("testfifo", O_WRONLY | O_NONBLOCK);
	if(fd == -1)
		sys_err("open error");

	//创建一个base
	struct event_base *base = event_base_new();

	//创建一个事件
	struct event *ev = NULL;

	//写一次
	//ev = event_new(base, fd, EV_WRITE, write_cb, NULL);
	
	//持续写
	ev = event_new(base, fd, EV_WRITE | EV_PERSIST, write_cb, NULL);

	event_add(ev, NULL);

	event_base_dispatch(base);

	event_base_free(base);

	return 0;
}
