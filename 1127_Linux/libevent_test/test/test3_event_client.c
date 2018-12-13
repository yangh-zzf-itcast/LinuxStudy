/*************************************************************************
    > File Name: test3_event_client.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Thu 29 Nov 2018 07:13:55 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/stat.h>
#include<fcntl.h>

int main(int argc, char *argv[])
{
	int fd = 0;

	const char *str = "hello event";

	fd = open("event.fifo", O_RDWR);

	while(1)
	{
		write(fd, str, strlen(str));
		sleep(1);
	}

	close(fd);

	return 0;
}
