//基于libevent 框架的 web服务器实现

#ifndef _LIB_SERVER_H_
#define _LIB_SERVER_H_

//信号回调函数
void signal_cb(evutil_socket_t sig, short events, void *user_data);

//监听器回调函数
void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
								 struct sockaddr *sa, int socklen, void *user_data);

void conn_readcb(struct bufferevent *bev, void *user_data);

void conn_eventcb(struct bufferevent *bev, short events, void *user_data);

//浏览器发送给服务器的请求
int response_http(struct bufferevent *bev, char *path);

//服务器给浏览器发送 请求的普通文件的 数据内容
void send_file(struct bufferevent *bev, const char *file);

//服务器给浏览器发送的 目录信息
void send_dir(struct bufferevent *bev, const char *dirname);

//给浏览器发送http应答协议， 使用bufferevent_write 函数来向缓冲区写数据
void send_respond_header(struct bufferevent *bev, int no, char *disp, char *type, int len);

//404 错误页面的 html格式数据
void send_error(struct bufferevent *bev, int status, char *title, char *text);

#endif
