/*************************************************************************
    > File Name: test5_event_callback_server.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Thu 29 Nov 2018 07:39:19 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<ctype.h>

#include<arpa/inet.h>

#include<event2/listener.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<error.h>

#define MAX_LINE 256
//读回调函数
static void
echo_read_cb(struct bufferevent *bev, void *ctx)
{
	//如果客户端有数据歇过来，那么会触发当前的回调函数
	struct evbuffer *input = bufferevent_get_input(bev);
	//input 就是当前bufferevent的输入缓存区地址，如果想得到用户端数据就从input中取获取
	//
	//从input 得到 客户端写来内容
	//小写------》大写
	//然后放入到写缓存中
	
	/*struct evbuffer *tmp = evbuffer_new();
	evbuffer_add_buffer(tmp, input);
	
	unsigned char* p = evbuffer_pullup(tmp,evbuffer_get_length(tmp));
	*/
	int n = evbuffer_get_length(input);
	char p[MAX_LINE];	
	bufferevent_read(bev, p ,n);
    int i;
	for(i = 0;i < n;++i)
	{
		*(p + i) = toupper(*(p + i));
	}
	
	struct evbuffer *output = bufferevent_get_output(bev);
	evbuffer_add(output, p, n);

	//将读缓存中 input 的数据放入写缓存 output
	//evbuffer_add_buffer(output, input);
    //evbuffer_free(tmp);
}


static void
echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
	if(events & BEV_EVENT_ERROR)
		perror("Error from bufferevent");
	if(events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
	{
		bufferevent_free(bev);		
	}	
}

//接受回调函数
static void
accept_conn_cb(struct evconnlistener *listener,
			evutil_socket_t fd, struct sockaddr *address, int socklen, 
			void *ctx)
{
	//fd ------ 服务器已经accept成功的cfd ， fd就是可以直接跟客户端 通信的套接字	
	
	//得到一个新的连接
	struct event_base *base = evconnlistener_get_base(listener);
	
	//创建一个bufferevent事件  绑定fd 和base
	struct bufferevent *bev = bufferevent_socket_new(
					base, fd, BEV_OPT_CLOSE_ON_FREE);
	
	//当前刚创建好的bufferevent事件， 注册一些回调函数
	//这里注册了写 回调函数 和 事件回调函数
	bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);
	
	//启动监听buffreevent的 读事件 和 写事件
	bufferevent_enable(bev, EV_READ|EV_WRITE);
}

//错误回调函数
static void 
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
	//得到一个新的连接
  struct event_base *base = evconnlistener_get_base(listener);
  int err = EVUTIL_SOCKET_ERROR();
  fprintf(stderr, "Got an error %d (%s) on the listener. Shutting down.\n",
  				err, evutil_socket_error_to_string(err));
	//跳出循环
	event_base_loopexit(base, NULL);
}

int main(int argc, char *argv[])
{

  //base
  struct event_base *base;
	
  struct evconnlistener *listener;
  
  //套接字地址
  struct sockaddr_in sin;
  
  int port = 9876;
  
  if(argc > 1)
  {
  	port = atoi(argv[1]);		
  }
  if(port <=0 || port > 65535)
  {
  	puts("Invalid port");
  	return 1;	
  }
  

  //以下是固定的用法，可以直接套用
  //创建base，设置套接字，给listener封装事件
  	
   //创建一个 eventbase 句柄， 在内核开辟一个监听事件的根节点
   base = event_base_new();
   if(!base)
   {
   		puts("Couldn't open event base");
   		return 1;	
   }
   
   //使用前清空套接字结构体，防止有另外的特定平台字段搞乱我们
   memset(&sin, 0, sizeof(sin));
   //INET地址，IPv4
   sin.sin_family = AF_INET;
   
   //监听0.0.0.0
   sin.sin_addr.s_addr = htonl(0);
   //监听所给端口
   sin.sin_port = htons(port);
   
   //将listen_fd 分装一个事件，  该函数默认在里面已经执行了bind 和 listen指令
   listener = evconnlistener_new_bind(base, accept_conn_cb, NULL,
   				LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE, -1
   				,(struct sockaddr*)&sin, sizeof(sin));
   
   if(!listener)
   {
   		perror("Couldn't create listener");
   		return 1;		
   }
   
   //错误回调函数
   evconnlistener_set_error_cb(listener, accept_error_cb);
   
   //开始循环监听事件
   event_base_dispatch(base);
   
   return 0;
}
