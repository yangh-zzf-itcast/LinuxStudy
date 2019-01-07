/*************************************************************************
    > File Name: baseFun.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Mon 07 Jan 2019 06:55:53 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <strings.h>
#include <sys/stat.h>

#include "baseFun.h"

//16进制数转化为10进制, return 0不会出现 
int hexit(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0;	
}

//中文字符解码
/* 这里的内容是处理%20之类的东西！是"解码"过程。
   %20 URL编码中的‘ ’(space)
   %21 '!' %22 '"' %23 '#' %24 '$'
   %25 '%' %26 '&' %27 ''' %28 '('......*
   相关知识:html中的‘ ’(space)是 */
void decode_str(char *to, char *from)
{
	for ( ; *from != '\0'; ++to, ++from) {
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
			 *to = hexit(from[1])*16 + hexit(from[2]);
		     from += 2;//移过已经处理的两个字符(%21指针指向1),表达式3的++from还会再向后移一个字符
		 } else{
			 *to = *from;
		 }
	}
	*to = '\0';
}

//"编码"，用作回写浏览器的时候，将除字母数字及/_.-~以外的字符转义后回写。
//strencode(encoded_name, sizeof(encoded_name), name);
void encode_str(char* to, size_t tosize, const char* from)
{
	int tolen;
	//一个汉字4个字节
    for (tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from) {
		if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0) {
			*to = *from;
			++to;
			++tolen;
		} else {
		sprintf(to, "%%%02x", (int) *from & 0xff);	
		to += 3;			
		tolen += 3;			
		}								
	}	
	*to = '\0';																				
}

//获取浏览器请求的文件的类型
const char *get_file_type(const char *name)
{
	char *dot;

	//hello.txt
	//从左向右找 . 字符，如果不存在就返回NULL
	dot = strrchr(name, '.');		//将字符串指针定位到要查找的符号上，并返回后续的字符
	if(dot == NULL)
		return "Content-Type:text/plain;charset=utf-8";
	if(strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "Content-Type:text/html;charset=utf-8";
	if(strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
		return "Content-Type:image/jpeg";
	if(strcmp(dot, ".gif") == 0)
		return "Content-Type:image/gif";
	if(strcmp(dot, ".png") == 0)
		return "Content-Type:image/png";
	if(strcmp(dot, ".css") == 0)
		return "Content-Type:text/css";
	if(strcmp(dot, ".au") == 0)
		return "Content-Type:audio/basic";
	if(strcmp(dot, ".wav") == 0)
		return "Content-Type:audio/wav";
	if(strcmp(dot, ".avi") == 0)
		return "Content-Type:audio/wav";
	if(strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "Content-Type:video/quicktime";
	if(strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "Content-Type:video/mpeg";
	if(strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "Content-Type:model/vrml";
	if(strcmp(dot, ".mp3") == 0)
		return "Content-Type:audio/mpeg";

	return "Content-Type:text/plain;charset=utf-8";
}


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

