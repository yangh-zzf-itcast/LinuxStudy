/*************************************************************************
    > File Name: test3_event_server.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Thu 29 Nov 2018 06:47:19 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/stat.h>
#include<fcntl.h>

#include<event2/event.h>

void callback_func(evutil_socket_t fd, short event, void *arg)
{
	char buf[256] = {0};
	int len = 0;
	struct event_base *base = (struct event_base *)arg;

	printf("fd = %d, event = %d\n", fd, event);
	len = read(fd, buf, sizeof(buf));

	if(len == -1)
	{
		perror("read"); return;
	}else if(len == 0)
	{
		perror("remote close fd"); return;
	}else
	{
		buf[len] = '\0';
		printf("read buf = [%s]\n", buf);
		FILE *fp = fopen("event_base_stat.txt", "a");
		if(fp == NULL)
		{
			perror("fopen err");
			exit(1);
		}

		event_base_dump_events(base, fp);
		fclose(fp);
	}
	return;
}

int main(int argc, char *argv[])
{
	struct event_base *base = NULL;
	struct event *evfifo = NULL;
	const char *fifo = "event.fifo";

	int fd;

	unlink(fifo);
	if(mkfifo(fifo, 0644) == -1)
	{
		perror("mkfifo");
		exit(1);
	}

	fd = open(fifo, O_RDONLY);
	if(fd == -1)
	{
		perror("open socket error");
		exit(1);
	}

	//1.create
	base = event_base_new();

	//2.bind
	evfifo = event_new(base, fd, EV_READ|EV_PERSIST, callback_func, base);
	
	//3.add
	event_add(evfifo, NULL);

	//4.listen
	event_base_dispatch(base);

	//5.free
	event_free(evfifo);
	event_base_free(base);

	return 0;	
}
