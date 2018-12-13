/*************************************************************************
    > File Name: test2_event.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Wed 28 Nov 2018 01:25:25 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fcntl.h>
#include<event2/event.h>

//call back function
void cb_func(evutil_socket_t fd, short what, void *arg)
{
	const char *data = arg;
	printf("Got an event on socket %d:%s%s%s%s  [%s]\n", 
			(int) fd,
			(what&EV_TIMEOUT) ? "timeout" : "",
			(what&EV_READ) ? "read" : "",
			(what&EV_WRITE) ? "write" : "",
			(what&EV_SIGNAL) ? "signal" : "",
			data);
}

//loop function
void main_loop(evutil_socket_t fd1, evutil_socket_t fd2)
{
	struct event *ev1, *ev2;
	struct timeval five_seconds = {5, 0};
	struct event_base *base = event_base_new();
	/*The caller has set up fd1 and fd2, and make them nonblocking. */
	
	//监听fd1是否可读，设置超时/持续监听
	ev1 = event_new(base, fd1, EV_TIMEOUT|EV_READ|EV_PERSIST
					, cb_func, (char*) "Reading event");
	//listen if fd2 can be writed ,decide-->not decide 
	//监听fd2是否可写，并且自动将非未决转化为未决状态
	/*ev2 = event_new(base, fd2, EV_WRITE|EV_PERSIST     事件处于 初始状态
                    , cb_func, (char*) "Writing event");*/
    ev2 = event_new(base, fd2, EV_WRITE
                    , cb_func, (char*) "Writing event");   
	//insert event on base
	//调用结束 事件处于 未决状态
	event_add(ev1, &five_seconds);
	event_add(ev2, NULL);
	
	//监听 安插ev1 和ev2 的base，当对应事件满足时，激活状态, 直接调用回调函数处理
	event_base_dispatch(base);
	//最后回调结束  事件变为非未决状态
}

int main(int argc, char *argv[])
{
	const char *fifo = "event.fifo";
	int fd1, fd2;

	unlink(fifo);   //make sure fifo does not exact

	//生成fifo有名管道
	if(mkfifo(fifo, 0644))
	{
		perror("mkfifo error");
		exit(1);
	}
	fd1 = open(fifo, O_RDONLY|O_NONBLOCK, 0644);
	if(fd1 < 0)
	{
		perror("open fifo error");
		exit(1);
	}

	fd2 = open(fifo, O_WRONLY|O_NONBLOCK, 0644);
	if(fd2 < 0)
	{
		perror("open fifo error");
		exit(1);
	}	

	main_loop(fd1, fd2);	
	
	close(fd1);
	close(fd2);

	return 0;
}

