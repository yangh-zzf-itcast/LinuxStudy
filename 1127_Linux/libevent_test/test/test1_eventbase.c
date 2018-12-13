/*************************************************************************
    > File Name: test1_eventbase.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Tue 27 Nov 2018 10:03:53 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>

#include<event2/event.h>

int main(int argc, char *argv[])
{
	struct event_base *base;  
	//create base object
	base = event_base_new();

	//get OS supported IO mechanism
	int i;
	const char **methods = event_get_supported_methods();
	printf("Starting Libevent %s. Available methods are :\n", event_get_version());
	for(i = 0;methods[i] != NULL;++i)
	{
		printf(" %s\n", methods[i]);
	}

	//listen event which bind on base
	event_base_dispatch(base);     //= while(1) + epoll_wait(), xunhuan self
		

	//free base
	event_base_free(base);

	return 0;
}
