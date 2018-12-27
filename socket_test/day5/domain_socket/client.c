/*************************************************************************
    > File Name: client.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Thu 27 Dec 2018 06:22:20 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<strings.h>
#include<ctype.h>
#include<arpa/inet.h>
#include<sys/un.h>		/* un */
#include<stddef.h>

#include"wrap.h"

#define SERV_ADDR "serv.socket"
#define CLIE_ADDR "clie.socket"

/* 与网络C/S模型客户端的区别是 客户端和服务器的地址结构都需要进行初始化 */

int main(void)
{
	int cfd, len;
	char buf[BUFSIZ];
	struct sockaddr_un servaddr, cliaddr;		/* 本地套接字 地址结构 16位头 + socket伪文件 */

	cfd = Socket(AF_UNIX, SOCK_STREAM, 0);		/* AF_UNIX 是本地套接字的类型 */

	/* 本地套接字初始化 */
	bzero(&cliaddr, sizeof(cliaddr));
	cliaddr.sun_family = AF_UNIX;
	strcpy(cliaddr.sun_path, CLIE_ADDR);		/* socket伪文件的名字 包含文件路径 */

	/* offsetof 是内核函数，用来计算结构体中的成员变量与结构体的偏移量 */
	len = offsetof(struct sockaddr_un, sun_path) + strlen(cliaddr.sun_path);	/* 计算客户端地址的有效长度 */

	unlink(CLIE_ADDR);		/* 确保客户端的socket文件 不存在*/
	Bind(cfd, (struct sockaddr*)&cliaddr, len);

	/******* 下面对服务器 地址结构 初始化 ******/

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, SERV_ADDR);		/* socket伪文件的名字 包含文件路径 */

	len = offsetof(struct sockaddr_un, sun_path) + strlen(servaddr.sun_path);	/* 计算服务端地址的有效长度 */
	
	Connect(cfd, (struct sockaddr*)&servaddr, len);		/* 与服务端建立链接 */

	printf("客户端与服务端 的socket伪文件 建立链接 完成。。。\n");

	while(fgets(buf, sizeof(buf), stdin) != NULL){
		Write(cfd, buf, strlen(buf));
		len = read(cfd, buf, sizeof(buf));
		Write(STDOUT_FILENO, buf, len);
	}

	close(cfd);

	return 0;
}

