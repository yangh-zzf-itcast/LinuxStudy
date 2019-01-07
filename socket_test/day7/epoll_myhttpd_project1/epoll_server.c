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

//epoll���������
void epoll_run(int port)
{
	int i = 0;
	struct epoll_event all_events[MAXSIZE];		//��������¼�������

	//����һ��������������
	int epfd = epoll_create(MAXSIZE);
	if(epfd == -1){
		sys_err("epoll_create error");
	}

	//����lfd��������ӵ���������
	int lfd = init_listen_fd(port, epfd);

	while(1){
		//�����ڵ�Ķ�Ӧ�¼�
		int ret = epoll_wait(epfd, all_events, MAXSIZE, -1);
		if(ret == -1){
			sys_err("epoll_wait error");
		}

		for(i = 0;i < ret;i++){
			//ֻ������¼��������¼�Ĭ�ϲ�����
			struct epoll_event *pev = &all_events[i];
			
			//���Ƕ��¼�ֱ������
			if(!(pev->events & EPOLLIN)){
				continue;
			}
			if(pev->data.fd == lfd){
				do_accept(lfd, epfd);	//�����lfd�� ���ܿͻ�����������
			}else{
				do_read(pev->data.fd, epfd);	//������������ connfd��������
			}
		}
	}
}

//������lfd�ĳ�ʼ�������ҽ�lfd�ҵ����������epfd��
int init_listen_fd(int port, int epfd)
{
	//�����������׽���
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if(lfd == -1){
		sys_err("socket error");
	}

	struct sockaddr_in srv_addr;
	bzero(&srv_addr, sizeof(srv_addr));
	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(port);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	//�˿ڸ���
	int opt = 1;
	setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//��lfd�󶨵�ַ�ṹ
	int ret = bind(lfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
	if(ret == -1){
		sys_err("bind error");
	}

	ret = listen(lfd, 128);
	if(ret == -1){
		sys_err("listen error");
	}

	//��lfd�󶨵� epoll ����
	struct epoll_event ev;
	ev.data.fd = lfd;
	ev.events = EPOLLIN;

	ret = epoll_ctl(epfd, EPOLL_CTL_ADD, lfd, &ev);
	if(ret == -1){
		sys_err("epoll_ctl add lfd error");
	}

	return lfd;
}

//lfd ���������¼�
void do_accept(int lfd, int epfd)
{
	struct sockaddr_in clt_addr;
	socklen_t clt_addr_len = sizeof(clt_addr);

	int cfd = accept(lfd, (struct sockaddr *)&clt_addr, &clt_addr_len);
	if(cfd == -1){
		sys_err("accept error");
	}

	//��ӡ�ͻ��˵�IP �� PORT
	char client_ip[64] = {
		0
	};

	printf("New Client IP: %s, port: %d, cfd = %d\n",
			inet_ntop(AF_INET, &clt_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
			ntohs(clt_addr.sin_port), cfd);

	//����cfd ������ETģʽ ��Чģʽ
	int flag = fcntl(cfd, F_GETFL);
	flag |= O_NONBLOCK;
	fcntl(cfd, F_SETFL, flag);

	//���½ڵ�cfd�ҵ�epoll��������
	struct epoll_event ev;
	ev.data.fd = cfd;
	ev.events = EPOLLIN | EPOLLET;

	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
	if(ret == -1){
		sys_err("epoll_ctl add cfd error");
	}
}

//����������ж�д����
void do_read(int cfd, int epfd)
{
	//���ȶ�ȡ�����������һ��http����Э�飬���в�ֻ�ȡ�ļ�����Э��ŵ�
	//GET /hello.c HTTP/1.1 
	char line[1024] = {
		0
	};
	int len = get_line(cfd, line, sizeof(line));
	if(len == 0){
		printf("��������⵽������Ѿ��ر�....\n");
		disconnect(cfd, epfd);
	}else{
		printf("===========����ͷ==========\n");
		printf("����������: %s\n", line);

		//��ȡʣ������
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
		//�ر��׽��֣�Ȼ��cfd�Ӽ�����epfd��ժ��
		disconnect(cfd, epfd);
	}

}

//�������������http����
void http_request(int cfd, const char *request)
{
	//��sscanf �����Լ� ����ƥ�� ���ַ������в��
	char method[16], path[256], protocol[16];
	sscanf(request, "%[^ ] %[^ ] %[^ ]", method, path, protocol);
	printf("method=%s, path=%s, protocol=%s\n", method, path, protocol);
	
	//����������ļ���Ҫ������ʶ����������� -> ����
	//�������
	decode_str(path, path);

	char *file = path + 1;		//ȡ�����Ҫ���ʵ� �ļ���  /hello.c

	//���û��ָ�����ʵ���Դ����Ĭ����ʾ��ԴĿ¼�е�����
	if(strcmp(path, "/") == 0){
		file = "./";
	}

	int len = strlen(file);
	//����Ƿ�Ϊ�Ϸ����ļ���ʽ
	if (file[0] == '/' || strcmp(file, "..") == 0 
					   || strncmp(file, "../", 3) == 0 
		 			   || strstr(file, "/../") != NULL
					   || strcmp(&(file[len-3]), "/..") == 0){

	send_error(cfd, 400, "Bad Request", "Illegal filename.");					
	}							

	struct stat sbuf;

	//�ж��ļ��Ƿ����
	int ret = stat(file, &sbuf);
	if(ret != 0){
		//�ļ������ڼ�һ��404 not foundҳ��, ������һ������
		//sys_err("stat error");
		send_error(cfd, 404, "Not Found", "No such file or direntry");
		return;
	}

	if(S_ISREG(sbuf.st_mode)){			//�����һ����ͨ�ļ�
		//printf("-----it is a file\n");
		
		//��������������ط�httpЭ��Ӧ��
		//��ȡ�����������ļ�������
		const char *type =  get_file_type(file);
		send_respond(cfd, 200, "OK", (char *)type, sbuf.st_size);

		//��������������ط� �����������ļ�����������
		send_file(cfd, file);
	}else if(S_ISDIR(sbuf.st_mode)){	//������ʵ���һ��Ŀ¼
		send_respond(cfd, 200, "OK", (char *)get_file_type(".html"), -1);

		//����Ŀ¼����
		send_dir(cfd, file);
	}
}


//�����������httpӦ��Э��
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


//����������������� �������ͨ�ļ��� ��������
void send_file(int cfd, const char *file)
{
	int n = 0;
	char buf[1024] = {
		0
	};

	//�򿪷��������ص��ļ�
	int fd = open(file, O_RDONLY);
	if(fd == -1){
		//�Ӹ�404����ҳ��
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

//����������������͵� Ŀ¼��Ϣ
void send_dir(int cfd, const char *dirname)
{
	int i , ret;
	//ƴ��һ��htmlҳ��<table></table>
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

	//Ŀ¼��Ķ���ָ��
	struct dirent **ptr;
	//scandir ����������ǰĿ¼�µ������ļ�����Ŀ¼ ������ĸ�������򣬷��������ܸ���
	int num = scandir(dirname, &ptr, NULL, alphasort);

	//���� scandir�����Ĳ������б���
	for(i = 0;i < num;i++){
		char *name = ptr[i]->d_name;

		//ƴ���ļ�������·��
		sprintf(path, "%s/%s", dirname, name);
		printf("path = %s ===========\n", path);
		struct stat st;
		stat(path, &st);

		//�������ط�����������������ļ����б������ ���� -> ����
		encode_str(enstr, sizeof(enstr), name);

		if(S_ISREG(st.st_mode)){
			sprintf(buf + strlen(buf),	//tr��ʾһ����table���Ƕ�� td��ʾ����Ĳ˵���
					"<tr><td><a href = \"%s\">%s</a></td><td>%ld</td></tr>",
					enstr, name, (long)st.st_size);
		}else if(S_ISDIR(st.st_mode)){		//�����Ŀ¼�����ֺ�Ӹ� /
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
	//��ͳ�Ĵ�Ŀ¼�����ķ�ʽ
	DIR *dir = opendir(dirname);
	if(dir == NULL){
		sys_err("opendir error");
	}

	//��Ŀ¼
	struct dirent *ptr = NULL;
	while((ptr = readdir(dir)) != NULL){		//�ݹ������
		char *name = ptr->d_name;
	}
	closedir(dir);
#endif
}



//������˶Ͽ�����, ����epfd��������ժ��
void disconnect(int cfd, int epfd)
{
	int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, cfd, NULL);
	if(ret == -1){
		sys_err("epoll_ctl del cfd error");
	}
	close(cfd);
}

//404 ����ҳ��� html��ʽ����
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
