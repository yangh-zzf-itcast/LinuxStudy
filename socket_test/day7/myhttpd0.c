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

#define MAXSIZE 2048

//获取一行/r/n 结尾的数据
int get_line(int cfd, char *buf, int size)
{
	int i = 0;
	char c = '\0';
	int n;
	while((i < size - 1) && (c != '\n')){
		n = recv(cfd, &c, 1, 0);		//每次读走一个字节
		if(n > 0){
			if(c == '\r'){
				n = recv(cfd, &c, 1, MSG_PEEK);	//尝试读一个字节,数组仍然保留未读走
				if((n > 0) && (c =='\n')){
					recv(cfd, &c, 1, 0);
				}else{
					c = '\n';
				}
			}
			buf[i] = c;
			i++;
		}else{
			c = '\n';
		}
	}

	buf[i] = '\0';
	if(-1 == n)
		i = n;

	return i;
}

void sys_err(const char *arg)
{
	perror(arg);
	exit(1);
}

int init_listen_fd(int port, int epfd)
{
	//创建监听的套接字
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if(lfd == -1){
		sys_err("socket error");
	}

	struct sockaddr_in srv_addr;
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(port);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//端口复用
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//给lfd绑定地址结构
	int ret = bind(lfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
	if(ret == -1){
		sys_err("bind error");
	}

	ret = listen(lfd, 128);
	if(ret == -1){
		sys_err("listen error");
	}

	//将lfd绑定到 epoll 树上
	struct epoll_event ev;
	ev.data.fd = lfd;
	ev.events = EPOLLIN;

	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
	if(ret == -1){
		sys_err("epoll_ctl add lfd error");
	}

	return lfd;
}

//lfd 处理连接事件
void do_accept(int lfd, int epfd)
{
	struct sockaddr_in clt_addr;
	socklen_t clt_addr_len = sizeof(clt_addr);

	int cfd = accept(lfd, (struct sockaddr *)&clt_addr, &clt_addr_len);
	if(cfd == -1){
		sys_err("accept error");
	}

	//打印客户端的IP 和 PORT
	char client_ip[64] = {
		0
	};

	printf("New Client IP: %s, port: %d, cfd = %d\n",
			inet_ntop(AF_INET, &clt_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
			ntohs(clt_addr.sin_port), cfd);

	//设置cfd 非阻塞ET模式 高效模式
	int flag = fcntl(cfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(cfd, F_SETFL, flag);

	//将新节点cfd挂到epoll监听树上
	struct epoll_event ev;
	ev.data.fd = cfd;
	ev.events = EPOLLIN | EPOLLET;

	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
	if(ret == -1){
		sys_err("epoll_ctl add cfd error");
	}
}

//浏览器端断开链接, 并从epfd监听树上摘下
void disconnect(int cfd, int epfd)
{
	int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
	if(ret == -1){
		sys_err("epoll_ctl del cfd error");
	}
	close(cfd);
}

void http_request(int cfd, const char *file)
{
	struct stat sbuf;

	//判断文件是否存在
	int ret = stat(file, &sbuf);
	if(ret != 0){
		sys_err("stat error");
	}

	if(S_ISREG(sbuf.st_mode)){			//如果是一个普通文件
		printf("-----it is a file\n");
	}
}

//对浏览器进行读写操作
void do_read(int cfd, int epfd)
{
	//首先读取浏览器发过来一行http请求协议，进行拆分获取文件名、协议号等
	//GET /hello.c HTTP/1.1 
	char line[1024] = {
		0
	};
	int len = get_line(cfd, line, sizeof(line));
	if(len == 0){
		printf("服务器检测到浏览器已经关闭....\n");
		disconnect(cfd, epfd);
	}else{
		//用sscanf 函数以及 正则匹配 对字符串进行拆分
		char method[16], path[256], protocol[16];
		sscanf(line, "%[^ ] %[^ ] %[^ ]", method, path, protocol);
		printf("method=%s, path=%s, protocol=%s\n", method, path, protocol);

		//读取剩下数据
		while(1){
			char buf[1024] = {
				0
			};
			len = get_line(cfd, buf, sizeof(buf));
			if(buf[0] == '\n' || len == -1){
				break;
			}
		}
		if(strncasecmp(method, "GET", 3) == 0){
			char *file = path + 1;		//取浏览器要访问的 文件名  /hello.c
			http_request(cfd, file);
		}
	}
}

//epoll服务器框架
void epoll_run(int port)
{
	int i = 0;
	struct epoll_event all_events[MAXSIZE];		//满足监听事件的数组

	//创建一个监听树的树根
	int epfd = epoll_create(MAXSIZE);
	if(epfd == -1){
		sys_err("epoll_create error");
	}

	//创建lfd，并且添加到监听树上
	int lfd = init_listen_fd(port, epfd);

	while(1){
		//监听节点的对应事件
		int ret = epoll_wait(epfd, all_events, MAXSIZE, -1);
		if(ret == -1){
			sys_err("epoll_wait error");
		}

		for(i = 0;i < ret;i++){
			//只处理读事件，其他事件默认不处理
			struct epoll_event *pev = &all_events[i];
			
			//不是读事件直接跳过
			if(!(pev->events & EPOLLIN)){
				continue;
			}
			if(pev->data.fd == lfd){
				do_accept(lfd, epfd);	//如果是lfd， 接受客户端连接请求
			}else{
				do_read(pev->data.fd, epfd);	//如果是浏览器的 connfd，读数据
			}
		}
	}
}


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

