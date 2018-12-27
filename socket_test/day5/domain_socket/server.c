/*************************************************************************
    > File Name: server.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Thu 27 Dec 2018 06:46:59 PM CST
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
#include <sys/un.h>
#include <stddef.h>		/* offsetof 函数的头文件 */

#include "wrap.h"

#define SERV_ADDR "serv.socket"

int main(void)
{
	int lfd, cfd, len, size, i;
	struct sockaddr_un servaddr, cliaddr;
	char buf[4096];

	lfd = Socket(AF_UNIX, SOCK_STREAM, 0);

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sun_family = AF_UNIX;
	strcpy(servaddr.sun_path, SERV_ADDR);

	/* 服务器的 有效 地址结构 */
	len = offsetof(struct sockaddr_un, sun_path) + strlen(servaddr.sun_path);

	/* bind函数进行地址结构绑定 并会创建一个socket伪文件 即serv.socket*/
	/* 参数三的len 必须是有效的，不能直接用sizeof */
	Bind(lfd, (struct sockaddr *)&servaddr, len);

	Listen(lfd, 20);	//不重要

	printf("Wait Accept client......\n");

	while(1){
		len = sizeof(cliaddr);		/* 16位AF_UNIX大小 + 108字节 */

		/* 参数三 是传入传出参数 */
		cfd = Accept(lfd, (struct sockaddr *)&cliaddr, (socklen_t *)&len);

		/* 长度减去两个字节， 得到客户端 socket文件 名字的长度 */
		len -= offsetof(struct sockaddr_un, sun_path);
		cliaddr.sun_path[len] = '\0';

		printf("client bind success, filename[%s]\n", cliaddr.sun_path);

		while((size = Read(cfd, buf, sizeof(buf))) > 0){
			for(i = 0;i < size; i++){
				buf[i] = toupper(buf[i]);
			}
			Write(cfd, buf, size);
		}
		
		close(cfd);
	}

	close(lfd);

	return 0;
}
