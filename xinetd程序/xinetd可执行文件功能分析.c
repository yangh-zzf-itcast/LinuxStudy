#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define N 4096

//单文件版本

//发送http协议头
void send_headers(char *type)
{
	HTTP / 1.1 200 OK;
	Content - Type:Text / plain : charset = iso - 8859 - 1;
    Connection:close;
	\r\n
}

//发送错误号
void send_err(错误号，错误名称，错误描述)
{
	HTTP / 1.1 200 OK;
	Content - Type:Text / plain : charset = iso - 8859 - 1;
	\r\n
	<html>
		<head><title>错误号 错误名称</title></head>
    <body>
	错误描述
	</body>
	</html>
}

//主函数
//xinetd --> 执行./xhttpd==argv[0] 传参argv[1]==/home/yangh/xhttpdir==浏览器中的 '/',即服务器根目录
int main(int argc, char *argv[])
{
	char line[N];
	char method[N], path[N], protocol[N];

	char *file;
	struct stat sbuf;
	FILE *fp;
	int ich;

	//传参失败，报错
	if(argc !=2 )     // ./xhttpd   /home/yangh/dir
		//send_err();

    //更改工作目录到 /home/yangh/xhttpdir 下
	if(chdir(argv[1])==-1)
		//send_err();

    if(fgets(line,N,stdin)==NULL)    //GET /hello.c HTTP/1.1   ----可以用strtok解析，按空格分割
		//send_err();
	
	//拆分读到的第一行，将第二个参数文件路径 /hello.c 取出
	if(sscanf(line,"%[^ ] %[^ ] %[^ ]", method, path, protocol) != 3)    //拆分得到的方法，要访问的文件，以及HTTP协议头
																		 //用正则表达式的方法	%[^ ]
	   //send_err();
    
    //将剩余部分读完为止，/r/n结束
    while(fgets(line,N,stdin)!=NULL)
		if(strcmp(line,"\r\n"))
			break

	//判断使用的方法正确
	if(strcmp(methd, "GET")!=0)
	   //send_err()

    //将/hello.c 取出 hello.c
	file=path+1
    

    //*****此处开始判断读取的是文件还是目录，可以使用stat函数读取文件属性
    //使用stat函数判断文件是否存在
    if(stat(file,&sbuf)<0)
		//send_err

	//打开文件
	fp = fopen(file,"r")     //浏览器只读打开就可以了
    if(fp==NULL)
		//send_err


    //send_header

    while((ich=getc(fp))!=EOF)
		putchar(ich)
    
	//刷新缓存区
    fflush(stdout)

	fclose(fp)

    return 0
} 