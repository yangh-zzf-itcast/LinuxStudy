/*************************************************************************
    > File Name: wrap.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Mon 10 Dec 2018 09:21:07 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<unistd.h>
#include<signal.h>
#include<sys/socket.h>

//错误函数封装
void perr_exit(const char *s)
{
	perror(s);
	exit(1);
}

int Accept(int fd, struct sockaddr *sa, socklen_t *salenptr)
{
	int n;		//用于保留通信的socket文件描述符，作为返回值
again:
	if((n = accept(fd, sa, salenptr)) < 0){
		if((errno == EINTR) || (errno = ECONNABORTED))
			goto again;		//被信号终端　或者　连接中止　继续链接
		else
			perr_exit("accept error");
	}
	return n;
}

void Bind(int fd, const struct sockaddr *sa, socklen_t salen)
{
	int n;
	if((n = bind(fd, sa, salen)) < 0)
		perr_exit("bind error");
}

void Connect(int fd, const struct sockaddr *sa, socklen_t salen)
{
	int n;
	if((n = connect(fd, sa, salen)) < 0)
		perr_exit("connect error");
}

void Listen(int fd, int backlog)
{
	int n;
	if((n = listen(fd, backlog)) < 0)
		perror("listen error");
}

int Socket(int family, int type, int protocol)
{
	int n;		//保存创建的socket的文件描述符
	if((n = socket(family, type, protocol)) < 0)
		perr_exit("socket error");
	return n;
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
	ssize_t n;		//读到的字节数
again:
	if((n = read(fd, ptr, nbytes)) == -1){
		if(errno == EINTR)
			goto again;
		else
			return -1;
	}
	return n;
}

ssize_t Write(int fd, const void *ptr, size_t nbytes)
{
	ssize_t n;		//写入的字节数
again:
	if((n = write(fd, ptr, nbytes)) == -1){
		if(errno == EINTR)
			goto again;
		else
			return -1;
	}
	return n;
}


//fread 和 fwrite函数在socket编程中无法使用，因为他们是对文件进行操作
//而socket编程中只有文件描述符

/*参数3:应该读取的字节数*/   //socket 4096 readn(cfd, buf, 4096) nleft = 4096-1500
ssize_t Readn(int fd, void *vptr, size_t n)
{
	size_t  nleft;		//unsigned int剩余未读取的字节数 
	ssize_t nread;		//int  实际读到的字节数
	char	*ptr;

	ptr = vptr;
	nleft = n;			//n 未读取的字节数

	while(nleft > 0){
		if(nread == read(fd, ptr, nleft) < 0){
			if(errno == EINTR)
				nread = 0;
		}else if(nread == 0)
			break;
		
		nleft -= nread; //nleft = nleft - nread;
		ptr += nread;   //文件指针读多少移多少
	}

	return n - nleft;
}

ssize_t Writen(int fd, const void *vptr, size_t n)
{
	size_t		nleft;
	ssize_t		nwritten;
	const char  *ptr;

	ptr = vptr;
	nleft = n;
	while(nleft > 0){
		if((nwritten = write(fd, ptr, nleft)) <= 0){
			if(nwritten < 0 && errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}

	return n;
}
 
static ssize_t my_read(int fd, char *ptr)
{
	static int read_cnt;
	static char *read_ptr;
	static char read_buf[100];

	if(read_cnt <= 0){
again:
		if((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0){
			if(errno == EINTR)
				goto again;
			return -1;
		}else if(read_cnt == 0){
			return 0;
		}
		read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;

	return 1;
}

/*readline()  ---  fgets()*/
//传出参数vptr
ssize_t Readline(int fd, void *vptr, size_t maxlen)
{
	ssize_t n, rc;
	char	c, *ptr;
	ptr = vptr;

	for(n = 1;n < maxlen; n++){
		if((rc = my_read(fd, &c)) == 1){   //ptr[] = hello\n
			*ptr++ = c;
			if(c == '\n')
				break;
		}else if(rc == 0){
			*ptr = 0;
			return n-1;
		}else{
			return -1;
		}
	}
	*ptr = 0;

	return n;
}
