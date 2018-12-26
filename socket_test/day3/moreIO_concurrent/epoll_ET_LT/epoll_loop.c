/*************************************************************************
    > File Name: epoll_loop.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Sat 22 Dec 2018 10:03:17 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/epoll.h>
#include<time.h>
#include<arpa/inet.h>
#include<netinet/in.h>

#define MAX_EVENTS 1024		//监听上限数
#define BUFLEN 4096
#define SERV_PORT 8090

void recvdata(int fd, int events, void *arg);
void senddata(int fd, int events, void *arg);

//描述就绪的 文件描述符 的相关信息
//一般项目中不需要你自定义结构体，直接调用接口就行

struct myevent_s{
	int		fd;				//文件描述符
	int		events;			//对应的监听事件
	void	*arg;			//泛型参数
	void	(*call_back)(int fd, int events, void *arg);	//函数指针  回调函数
	int		status;			//是否在监听：1->在红黑树上监听；0->不在红黑树上监听
	char	buf[BUFLEN];
	int		len;
	long	last_active;	//记录每次加入红黑树的 g_efd 的时间
};

int g_efd;					//全局变量。返回epoll_create 返回的文件描述符
struct myevent_s g_events[MAX_EVENTS + 1];		//自定义结构体+1  ----> listenfd

/*	自定义结构体成员变量初始化	*/
/*	设置事件，以及事件对应的文件描述符 回调函数等 */
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void*), void *arg){
	ev->fd = fd;
	ev->call_back = call_back;
	ev->events = 0;
	ev->arg = arg;
	ev->status = 0;
	memset(ev->buf, 0, sizeof(ev->buf));	//bzero(ev->buf, sizeof(ev->buf));
	ev->len = 0;
	ev->last_active = time(NULL);		//调用eventset的事件 的函数

	return;
}

/* 向 epoll监听的红黑树 添加一个 文件描述符 */

//eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);
void eventadd(int efd, int events, struct myevent_s *ev)
{
	struct epoll_event epv = {0, {0}};
	int op;
	epv.data.ptr = ev;			//ptr对应原来的fd，现在fd存在ptr对应的结构体 ev 之中
	epv.events = ev->events = events;	//EPOLLIN 或者 EPOLLOUT

	if(ev->status == 0){		//未在红黑树上监听
		op = EPOLL_CTL_ADD;		//将其加入红黑树 g_efd，并将status设为1
		ev->status = 1;
	}

	if(epoll_ctl(efd, op, ev->fd, &epv) < 0)
		printf("event add failed [fd = %d], event[%d]\n", ev->fd, events);
	else
		printf("event add ok [fd = %d], op = %d, event[%0X]\n", ev->fd, op, events);

	return;
}

/* 从epoll 监听的红黑树中 删除一个文件描述符*/
void eventdel(int efd, int events, struct myevent_s *ev)
{
	struct epoll_event epv = {0, {0}};

	if(ev->status != 1)		//不在红黑树上
		return;

	epv.data.ptr = NULL;	//清空ptr对应的结构体
	ev->status = 0;			//修改文件描述符状态
	epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);	//从红黑树上摘下 ev->fd

	return;
}

/* 当有文件描述符就绪，lfd监听到有客户端链接，epoll返回，调用该函数与客户端建立链接 */
void acceptconn(int lfd, int events, void *arg)
{
	struct sockaddr_in cin;		//客户端地址
	socklen_t len = sizeof(cin);
	int cfd, i;

	if((cfd = accept(lfd, (struct sockaddr*)&cin, len)) == -1){
		if(errno != EAGAIN && errno != EINTR){
			printf("暂时不做出错处理");
		}
		printf("%s:accept, %s\n", __func__, strerror(errno));
		return;
	}

	/* 接受到cfd 客户端后 将cfd 设置事件，并绑定 回调函数 然后添加到g_events 数组的空闲位中*/
	do{
		for(i = 0;i < MAX_EVENTS;i++)
			if(g_events[i].status == 0)		//寻找g_events中的空位，将连接到的cfd放到这个位置
				break;						//类似与在select的自定义数组中寻找 -1 的位置，跳出for循环
	
		if(i == MAX_EVENTS){
			printf("%s:max connect limit[%d]\n", __func__, MAX_EVENTS);
			break;							//跳出do while语句，后续代码不执行
		}

		int flag = fcntl(cfd, F_GETFL);
		flag |= O_NONBLOCK;
		if((fcntl(cfd, F_SETFL, flag)) < 0){
			prinf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
			break;
		}

		/* 给cfd设置一个myevent_s的结构体， 回调函数设置为 recvdata 接收数据 */
		eventset(&g_events[i], cfd, recvdata, &g_events[i]);	//i 从 0开始 最大到MAX_EVENTS
		eventadd(g_efd, EPOLLIN, &g_events[i]);		//绑定在全局变量 g_efd 上

	}while(0)

	/**/
	printf("new connect [%s:%d][time:%ld], pos[%d]\n",
			inet_ntoa(cin.sin_addr), ntohs(cin.sin_port), g_event[i].last_active, i); 
	return;
}

/* 接受数据 */
void recvdata(int fd, int events, void *arg)
{
	struct myevent_s *ev = (struct myevent_s*)arg;
	int len;

	len = recv(fd, ev->buf, sizeof(ev->buf), 0);	//此时len = 0

	//将 回调函数为 recvdata 的fd 对应事件 从红黑树上摘
	eventdel(g_efd, ev);

	if(len > 0){
		ev->len = len;
		ev->buf[len] = '\0';
		printf("C[%d]:%s\n", fd, ev->buf);

		eventset(ev, fd, senddata, ev);		//将 fd 的回调函数设置为 senddata
		eventadd(g_efd, EPOLLOUT, ev)		//将事件设置为 监听写事件， 重新添加到监听红黑树g_efd
	}else if(len == 0){
		close(ev->fd);
		/* ev - g_events 地址相减 得到偏移元素位置 */
		printf("[fd = %s] pos[%ld], closed\n", fd, ev - g_events);
	}else{
		close(ev->fd);						//对端关闭，关闭链接
		printf("[fd = %d] pos[%ld], closed\n", fd, errno, strerror(error));
	}

	return;
}

/* 发送数据 */
void senddata(int fd, int events, void *arg)
{
	struct myevent_s *ev = (struct myevent_S *)arg;		//泛型指针强制类型转换
	int len;

	len = send(fd, ev->buf, ev->len, 0);

	//将 回调函数为 senddata 的fd 对应事件 从红黑树上摘除 
	eventdel(g_efd, ev);

	if(len > 0){
		printf("send[fd = %d], [%d]%s\n", fd, len, ev->buf);
		eventset(ev, fd, recvdata, ev);		//将 fd 的回调函数改为 recvdata
		eventadd(g_efd, EPOLLIN, ev);		//事件设置为监听读事件，重新添加到监听红黑树g_efd
	}else{									//关闭文件描述符, 关闭连接
		close(ev->fd);
		printf("send[fd = %d] error %s\n", fd, strerror);
	}

	return;
}

/* 创建socket ， 初始化lfd */
void initlistensocket(int efd, short port)
{
	struct sockaddr_in sin;
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);

	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	
	//设置lfd为非阻塞
	int flag = fcntl(lfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(lfd, F_SETFL, flag);

	bind(lfd, (struct sockaddr*)&sin, sizeof(sin));

	listen(lfd, 20);

	/* 重点！！绑定 文件描述符 和 回调函数 ,添加到对应的全局变量 自定义事件数组中*/
	/* 此处将 lfd 置于全局变量 g_events数组 的最后一个位置 */
	/*void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void*), void *arg)*/
	eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);

	/* 将绑定了 lfd 的事件 加到监听红黑树 efd 上监听读事件（有无客户端链接）*/
	/*void eventadd(int efd, int events, struct myevent_s *ev)*/
	eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);

	return;
}

int main(int argc, char *argv[])
{
	unsigned short port = SERV_PORT;

	if(argc == 2)
		port = atoi(argv[1])	//使用用户制定端口
	
	g_efd = epoll_create(MAX_EVENTS + 1);	//创建红黑树 返回给全局变量g_efd, 
											//+1的最后一个位置1025留给 服务器的 lfd
	initlistensocket(g_efd, port);			//初始化监听 socket 包含socket，bind，listen，accept

	struct epoll_event events[MAX_EVENTS + 1];	//保存已经满足就绪事件的 文件描述符数组
	printf("server running:port[%d]\n", port);

	int checkops = 0, i;
	
	//设置超时验证，每次测试100个链接，不测试listenfd，当客户端60s内没有和服务器通信，则关闭此客户端
	while(1){
		long now = time(NULL);		//取出系统当前时间
		for(i = 0;i < 100;i++, checkops++){		//一次循环100个，使用checkops控制检测对象
			if(checkops == MAX_EVENTS)		    //一轮检测结束
				checkops = 0;
			if(g_events[checkops].status) != 1)
				continue;

			long duration = now - g_events[checkops].last_active;	//用户不活跃的时间
			if(duration >= 60){
				close(g_events[checkops].fd);	//关闭不活跃客户端的链接，提高服务器的效率
				printf("[fd = %d] timeout\n",g_events[checkops].fd);
				eventdel(g_efd, &g_events[checkops]);
			}
		}

		/* 非阻塞监听 轮询*/
		/* 监听红黑树g_efd，将满足事件的文件描述符加到 events数组中，1s没有时间，返回0 */
		int nfd = epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);
		if(nfd < 0){
			printf("epoll_wait error, exit\n");
			break;
		}

		for(i = 0;i < nfd;i++){
			/* 原本是取出临时的sockfd 然后判断是lfd 还是其他的 cfd，现在是用ptr来判断 传入传出的是否一致 */
			/* 使用自定义结构体myevent_s 类型指针， 接受联合体data 的 void*ptr 成员*/
			/* ev 就相当于原来的 scokfd, 判断读事件还是写事件 */
			struct myevent_s *ev = (struct myevent_s *)events[i].data.ptr;

			/* 先执行读事件回调函数 再执行写事件回调函数 */

			//读事件就绪
			if((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)){
				/* call_back 为 recvdata */
				ev->call_back(ev->fd, events[i].events, ev->arg);
				//lfd EPOLLIN
			}

			//写事件就绪
			if((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT)){
				/* call_back 为 senddata */
				ev->call_back(ev->fd, events[i].events, ev->arg);
			}
		}

	}

	return 0;
}
