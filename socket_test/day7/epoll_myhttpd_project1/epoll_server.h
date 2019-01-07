/*************************************************************************
    > File Name: epoll_server.h
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Mon 07 Jan 2019 06:33:27 PM CST
 ************************************************************************/

#ifndef _EPOLL_SERVER_H_
#define _EPOLL_SERVER_H_

//基于epoll框架 简称的Web服务器

//epoll服务器框架
void epoll_run(int port);

//服务器lfd的初始化，并且将lfd挂到监听红黑树epfd上
int init_listen_fd(int port, int epfd);

//lfd 处理连接事件
void do_accept(int lfd, int epfd);

//对浏览器进行读写操作
void do_read(int cfd, int epfd);

//浏览器发过来的http请求
void http_request(int cfd, const char *request);

//给浏览器发送http应答协议
void send_respond(int cfd, int no, char *disp, char *type, int len);

//服务器给浏览器发送 请求的普通文件的 数据内容
void send_file(int cfd, const char *file);

//服务器给浏览器发送的 目录信息
void send_dir(int cfd, const char *dirname);

//浏览器端断开链接, 并从epfd监听树上摘下
void disconnect(int cfd, int epfd);

//404 错误页面的 html格式数据
void send_error(int cfd, int status, char *title, char *text);

#endif
