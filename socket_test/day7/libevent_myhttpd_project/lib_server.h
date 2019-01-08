//����libevent ��ܵ� web������ʵ��

#ifndef _LIB_SERVER_H_
#define _LIB_SERVER_H_

//�źŻص�����
void signal_cb(evutil_socket_t sig, short events, void *user_data);

//�������ص�����
void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
								 struct sockaddr *sa, int socklen, void *user_data);

void conn_readcb(struct bufferevent *bev, void *user_data);

void conn_eventcb(struct bufferevent *bev, short events, void *user_data);

//��������͸�������������
int response_http(struct bufferevent *bev, char *path);

//����������������� �������ͨ�ļ��� ��������
void send_file(struct bufferevent *bev, const char *file);

//����������������͵� Ŀ¼��Ϣ
void send_dir(struct bufferevent *bev, const char *dirname);

//�����������httpӦ��Э�飬 ʹ��bufferevent_write �������򻺳���д����
void send_respond_header(struct bufferevent *bev, int no, char *disp, char *type, int len);

//404 ����ҳ��� html��ʽ����
void send_error(struct bufferevent *bev, int status, char *title, char *text);

#endif
