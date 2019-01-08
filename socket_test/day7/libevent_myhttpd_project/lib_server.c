/*************************************************************************
    > File Name: lib_server.c
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
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <strings.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>
#include <signal.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/listener.h>

#include "lib_server.h"
#include "baseFun.h"

#define _HTTP_CLOSE_ "Connection: close\r\n"

//�źŻص�����
void signal_cb(evutil_socket_t sig, short events, void *user_data)
{
	struct event_base *base = (struct event_base *)user_data;
	struct timeval delay = {1, 0};	//1��
	
	printf("Caught an interrupt signal: exiting cleanly in one seconds.\n");
	event_base_loopexit(base, &delay);
}

//�������ص�����
void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
								 struct sockaddr *sa, int socklen, void *user_data)
{
	printf("************************begin call -------------%s\n", __FUNCTION__);
	struct event_base *base = (struct event_base *)user_data;
	struct bufferevent *bev;
	printf("client fd is %d\n", fd);
	
	bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
	
	if(!bev){
		fprintf(stderr, "Error constructing bufferevent!");
		event_base_loopbreak(base);
		return;
	}
	
	bufferevent_flush(bev, EV_READ | EV_WRITE, BEV_NORMAL);		//ˢ��һ�¶�д������
	bufferevent_setcb(bev, conn_readcb, NULL, conn_eventcb, NULL);	//���ö��ص����� �� �¼��ص�����
	bufferevent_enable(bev, EV_READ);		//��Ĭ�ϹرյĶ������
	
	printf("********************end call---------------%s\n", __FUNCTION__);
}


void conn_readcb(struct bufferevent *bev, void *user_data)
{
	printf("************************begin call -------------%s\n", __FUNCTION__);
	
	char buf[4096] = {0};
	bufferevent_read(bev, buf, sizeof(buf));		//�Ӷ������ж�ȡ����
	printf("buf[%s]\n", buf);
	
	//��sscanf �����Լ� ����ƥ�� ���ַ������в��
	char method[16], path[1024], protocol[16];
	sscanf(buf, "%[^ ] %[^ ] %[^ ]", method, path, protocol);
	printf("method=%s, path=%s, protocol=%s\n", method, path, protocol);
	
	if(strncasecmp(method, "GET", 3) == 0){
		response_http(bev, path);
	}
	printf("********************end call---------------%s\n", __FUNCTION__);

}

void conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
	printf("************************begin call -------------%s\n", __FUNCTION__);

	if(events & BEV_EVENT_EOF){
		printf("Connection closed.\n");
	}else if(events &BEV_EVENT_ERROR){
		printf("Got an error on the connection: %s\n", strerror(errno));
	}

	bufferevent_free(bev);
	
	printf("********************end call---------------%s\n", __FUNCTION__);
}

//��������͸�������������
int response_http(struct bufferevent *bev, char *path)
{
	//����������ļ���Ҫ������ʶ����������� -> ����
	//�������
	decode_str(path, path);

	char *file = path + 1;		//ȡ�����Ҫ���ʵ� �ļ���  /hello.c
	
	//���û��ָ�����ʵ���Դ����Ĭ����ʾ��ԴĿ¼�е�����
	if(strcmp(path, "/") == 0 || strcmp(path, "/.") == 0){
		file = "./";
	}
	
	int len = strlen(file);
	//����Ƿ�Ϊ�Ϸ����ļ���ʽ, �ݴ���
	if (file[0] == '/' || strcmp(file, "..") == 0 
					   || strncmp(file, "../", 3) == 0 
		 			   || strstr(file, "/../") != NULL
					   || strcmp(&(file[len-3]), "/..") == 0){

	send_error(bev, 400, "Bad Request", "Illegal filename.");	
	return 1;				
	}							
	
	struct stat sbuf;

	//�ж��ļ��Ƿ����
	int ret = stat(file, &sbuf);
	if(ret != 0){
		//�ļ������ڼ�һ��404 not foundҳ��, ������һ������
		//sys_err("stat error");
		send_error(bev, 404, "Not Found", "No such file or direntry");
		return 1;
	}
	
	if(S_ISREG(sbuf.st_mode)){			//�����һ����ͨ�ļ�
		//printf("-----it is a file\n");
		
		//��������������ط�httpЭ��Ӧ��
		//��ȡ�����������ļ�������
		const char *type =  get_file_type(file);
		send_respond_header(bev, 200, "OK", (char *)type, sbuf.st_size);

		//��������������ط� �����������ļ�����������
		send_file(bev, file);
	}else if(S_ISDIR(sbuf.st_mode)){	//������ʵ���һ��Ŀ¼
		send_respond_header(bev, 200, "OK", (char *)get_file_type(".html"), -1);

		//����Ŀ¼����
		send_dir(bev, file);
	}
	
	return 0;
}


//����������������� �������ͨ�ļ��� ��������
void send_file(struct bufferevent *bev, const char *file)
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
		send_error(bev, 404, "Not Found", "No such file or direntry");
		return;
	}

	while((n = read(fd, buf, sizeof(buf))) > 0){
		int ret = bufferevent_write(bev, buf, strlen(buf));
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
void send_dir(struct bufferevent *bev, const char *dirname)
{
	int i;
	char encoded_name[1024], path[1024], timestr[64];
	char buf[4096] = {0};
	
	struct dirent **dirinfo;
	struct stat st;
	
	sprintf(buf, "<html><head><meta charset=\"utf-8\"><title>Index of: %s</title></head>", dirname);
	sprintf(buf + strlen(buf), "<body bgcolor = \"#99cc99\"><h1>Index of: %s</h1><table>", dirname);
	
	//scandir ����������ǰĿ¼�µ������ļ�����Ŀ¼ ������ĸ�������򣬷��������ܸ���
	int num = scandir(dirname, &dirinfo, NULL, alphasort);
	//int num = scandir(dirname, &dirinfo, NULL, versionsort);
	
	//���� scandir�����Ĳ������б���
	for(i = 0;i < num;i++){
		char *name = dirinfo[i]->d_name;
		//�������ط�����������������ļ����б������ ���� -> ����
		encode_str(encoded_name, sizeof(encoded_name), name);
		
		//ƴ���ļ�������·��
		sprintf(path, "%s/%s", dirname, encoded_name);
		printf("path = %s ===========\n", path);

		//�������ط�����������������ļ����б������ ���� -> ����
		//encode_str(encoded_name, sizeof(encoded_name), name);

		if(stat(path, &st) < 0){
			sprintf(buf + strlen(buf),	//tr��ʾһ����table���Ƕ�� td��ʾ����Ĳ˵���
					"<tr><td><a href = \"%s\">%s</a></td></tr>\n",
					encoded_name, name);
		}else{
			strftime(timestr, sizeof(timestr),		//��ȡ�ļ������޸ĵ�ʱ��
							 "		%d	%b	%Y	%H:%M", localtime(&st.st_mtime));
			
			if(S_ISREG(st.st_mode)){
				sprintf(buf + strlen(buf),	//tr��ʾһ����table���Ƕ�� td��ʾ����Ĳ˵���
								"<tr><td><a href = \"%s\">%s</a></td><td>%s</td><td>%ld</td></tr>",
								encoded_name, name, timestr, (long)st.st_size);
			}else if(S_ISDIR(st.st_mode)){		//�����Ŀ¼�����ֺ�Ӹ� /
				sprintf(buf + strlen(buf), 
								"<tr><td><a href = \"%s/\">%s/</a></td><td>%s</td><td>%ld</td></tr>",
								encoded_name, name, timestr, (long)st.st_size);
			}
		}	
		
		bufferevent_write(bev, buf, strlen(buf));
		memset(buf, 0, sizeof(buf));
	}
	
	sprintf(buf + strlen(buf), "</table></body></html>");
	bufferevent_write(bev, buf, strlen(buf));
	printf("******************dir message send OK !!!-----------------\n");
}


//�����������httpӦ��Э�飬 ʹ��bufferevent_write �������򻺳���д����
void send_respond_header(struct bufferevent *bev, int no, char *disp, char *type, int len)
{
	char buf[1024] = {
		0
	};
	sprintf(buf, "HTTP/1.1 %d %s\r\n", no, disp);
	bufferevent_write(bev, buf, strlen(buf));
	
	sprintf(buf + strlen(buf), "%s\r\n", type);
	bufferevent_write(bev, buf, strlen(buf));
	
	sprintf(buf + strlen(buf), "Content-Length:%d\r\n", len);
	bufferevent_write(bev, buf, strlen(buf));
	
	//Connection: close
	bufferevent_write(bev, _HTTP_CLOSE_, strlen(_HTTP_CLOSE_));
	
	//send /r/n
	bufferevent_write(bev, "\r\n", 2);
}


//404 ����ҳ��� html��ʽ����
void send_error(struct bufferevent *bev, int status, char *title, char *text)
{
	send_respond_header(bev, 404, "File Not Found", "text/html", -1);
	
	char buf[4096] = {
		0
	};

	//senf_file(bev, "404.html");
	sprintf(buf, "<html><head><title>%d %s</title></head>\n", status, title);
	sprintf(buf + strlen(buf), "<body bgcolor = \"#cc99cc\"><h4 align=\"center\">%d %s</h4>\n", status, title);
	sprintf(buf + strlen(buf), "%s\n", text);
	sprintf(buf + strlen(buf), "<hr>\n</body>\n<html>\n");
	bufferevent_write(bev, buf, strlen(buf));

	return;
}



