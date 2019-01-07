/*************************************************************************
    > File Name: epoll_server.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Mon 07 Jan 2019 06:53:08 PM CST
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
#include <errno.h>
#include <dirent.h>

#include "epoll_server.h"
#include "baseFun.h"

#define MAXSIZE 2048

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

//服务器lfd的初始化，并且将lfd挂到监听红黑树epfd上
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
		printf("===========请求头==========\n");
		printf("请求行数据: %s\n", line);

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
	}

	if(strncasecmp("get", line, 3) == 0){
		http_request(cfd, line);
		//关闭套接字，然后将cfd从监听树epfd上摘下
		disconnect(cfd, epfd);
	}

}

//浏览器发过来的http请求
void http_request(int cfd, const char *request)
{
	//用sscanf 函数以及 正则匹配 对字符串进行拆分
	char method[16], path[256], protocol[16];
	sscanf(request, "%[^ ] %[^ ] %[^ ]", method, path, protocol);
	printf("method=%s, path=%s, protocol=%s\n", method, path, protocol);
	
	//浏览器请求文件，要将不能识别的中文乱码 -> 中文
	//解码操作
	decode_str(path, path);

	char *file = path + 1;		//取浏览器要访问的 文件名  /hello.c

	//如果没有指定访问的资源，则默认显示资源目录中的内容
	if(strcmp(path, "/") == 0){
		file = "./";
	}

	int len = strlen(file);
	//检测是否为合法的文件格式
	if (file[0] == '/' || strcmp(file, "..") == 0 
					   || strncmp(file, "../", 3) == 0 
		 			   || strstr(file, "/../") != NULL
					   || strcmp(&(file[len-3]), "/..") == 0){

	send_error(cfd, 400, "Bad Request", "Illegal filename.");					
	}							

	struct stat sbuf;

	//判断文件是否存在
	int ret = stat(file, &sbuf);
	if(ret != 0){
		//文件不存在加一个404 not found页面, 监听下一次请求
		//sys_err("stat error");
		send_error(cfd, 404, "Not Found", "No such file or direntry");
		return;
	}

	if(S_ISREG(sbuf.st_mode)){			//如果是一个普通文件
		//printf("-----it is a file\n");
		
		//服务器给浏览器回发http协议应答
		//获取浏览器请求的文件的类型
		const char *type =  get_file_type(file);
		send_respond(cfd, 200, "OK", (char *)type, sbuf.st_size);

		//服务器给浏览器回发 浏览器请求的文件的数据内容
		send_file(cfd, file);
	}else if(S_ISDIR(sbuf.st_mode)){	//如果访问的是一个目录
		send_respond(cfd, 200, "OK", (char *)get_file_type(".html"), -1);

		//发送目录内容
		send_dir(cfd, file);
	}
}


//给浏览器发送http应答协议
void send_respond(int cfd, int no, char *disp, char *type, int len)
{
	char buf[1024] = {
		0
	};
	sprintf(buf, "HTTP/1.1 %d %s\r\n", no, disp);
	sprintf(buf + strlen(buf), "%s\r\n", type);
	sprintf(buf + strlen(buf), "Content-Length:%d\r\n", len);
	send(cfd, buf, strlen(buf), 0);
	send(cfd, "\r\n", 2, 0);
}


//服务器给浏览器发送 请求的普通文件的 数据内容
void send_file(int cfd, const char *file)
{
	int n = 0;
	char buf[1024] = {
		0
	};

	//打开服务器本地的文件
	int fd = open(file, O_RDONLY);
	if(fd == -1){
		//加个404错误页面
		//sys_err("open error");
		send_error(cfd, 404, "Not Found", "No such file or direntry");
		return;
	}

	while((n = read(fd, buf, sizeof(buf))) > 0){
		int ret = send(cfd, buf, n, 0);
		if(ret == -1){
			if(errno == EAGAIN){
				printf("-----------EAGAIN\n");
			}else if(errno == EINTR){
				printf("-----------EINTR\n");
			}else{
				sys_err("send error");
			}
		}
	}

	close(fd);
}

//服务器给浏览器发送的 目录信息
void send_dir(int cfd, const char *dirname)
{
	int i , ret;
	//拼接一个html页面<table></table>
	char buf[4096] = {
		0
	};

	sprintf(buf, "<html><head><title>Index of: %s</title></head>", dirname);
	sprintf(buf + strlen(buf), "<body bgcolor = \"#99cc99\"><h1>Index of: %s</h1><table>", dirname);

	char enstr[1024] = {
		0
	};
	char path[1024] = {
		0
	};

	//目录项的二级指针
	struct dirent **ptr;
	//scandir 函数遍历当前目录下的所有文件和子目录 并以字母进行排序，返回所有总个数
	int num = scandir(dirname, &ptr, NULL, alphasort);

	//利用 scandir传出的参数进行遍历
	for(i = 0;i < num;i++){
		char *name = ptr[i]->d_name;

		//拼接文件的完整路径
		sprintf(path, "%s/%s", dirname, name);
		printf("path = %s ===========\n", path);
		struct stat st;
		stat(path, &st);

		//服务器回发给浏览器，对中文文件进行编码操作 中文 -> 乱码
		encode_str(enstr, sizeof(enstr), name);

		if(S_ISREG(st.st_mode)){
			sprintf(buf + strlen(buf),	//tr表示一行是table表格嵌套 td表示里面的菜单项
					"<tr><td><a href = \"%s\">%s</a></td><td>%ld</td></tr>",
					enstr, name, (long)st.st_size);
		}else if(S_ISDIR(st.st_mode)){		//如果是目录，名字后加个 /
			sprintf(buf + strlen(buf), 
					"<tr><td><a href = \"%s/\">%s/</a></td><td>%ld</td></tr>",
					enstr, name, (long)st.st_size);
		}

		ret = send(cfd, buf, strlen(buf), 0);
		if(ret == -1){
			if(errno == EAGAIN){
				perror("send error:");
				continue;
			}else if(errno == EINTR){
				perror("send error:");
				continue;
			}else{
				sys_err("send error:");
			}
		}
		memset(buf, 0, sizeof(buf));
	}

	sprintf(buf + strlen(buf), "</table></body></html>");
	send(cfd, buf, strlen(buf), 0);
	
	printf("dir message send OK !!!\n");

#if 0
	//传统的打开目录遍历的方式
	DIR *dir = opendir(dirname);
	if(dir == NULL){
		sys_err("opendir error");
	}

	//读目录
	struct dirent *ptr = NULL;
	while((ptr = readdir(dir)) != NULL){		//递归遍历读
		char *name = ptr->d_name;
	}
	closedir(dir);
#endif
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

//404 错误页面的 html格式数据
void send_error(int cfd, int status, char *title, char *text)
{
	char buf[4096] = {
		0
	};
	sprintf(buf, "%s %d %s\r\n", "HTTP/1.1", status, title);
	sprintf(buf + strlen(buf), "Content-Type:%s;charset=utf-8\r\n", "text/html");
	sprintf(buf + strlen(buf), "Content-Length:%d\r\n", -1);
	sprintf(buf + strlen(buf), "Connection:close\r\n");
	send(cfd, buf, strlen(buf), 0);
	send(cfd, "\r\n", 2, 0);
	
	memset(buf, 0, sizeof(buf));

	sprintf(buf, "<html><head><title>%d %s</title></head>\n", status, title);
	sprintf(buf + strlen(buf), "<body bgcolor = \"#cc99cc\"><h4 align=\"center\">%d %s</h4>\n", status, title);
	sprintf(buf + strlen(buf), "%s\n", text);
	sprintf(buf + strlen(buf), "<hr>\n</body>\n<html>\n");
	send(cfd, buf, strlen(buf), 0);

	return;
}
