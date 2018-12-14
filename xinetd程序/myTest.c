#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>

#define N 4096

//单文件版本

//发送http协议头
void send_header(int status, char* title, char *type);


//发送错误号
void send_err(int status, char* title, char* describle);

//判断文件类型
char* get_filetype(const char* file);


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
	if (argc != 2)     // ./xhttpd   /home/yangh/dir
	{
		send_err(500, "server error", "argv <2");
	}

	//更改工作目录到 /home/yangh/xhttpdir 下
	if (chdir(argv[1]) == -1)
	{
		send_err(500, "server error", "chdir error");
	}

	//GET /hello.c HTTP/1.1   ----可以用strtok解析，按空格分割
	if (fgets(line, N, stdin) == NULL)
	{
		send_err(500, "server error", "type path protocol");
	}

	//拆分读到的第一行，将第二个参数文件路径 /hello.c 取出
	if (sscanf(line, "%[^ ] %[^ ] %[^ ]", method, path, protocol) != 3)    //拆分得到的方法，要访问的文件，以及HTTP协议头
	{																	   //用正则表达式的方法	%[^ ]
		send_err(400, "browser error", "bad request");
	}

	//将剩余部分读完为止，/r/n结束
	while (fgets(line, N, stdin) != NULL)
	{
		if (strcmp(line, "\r\n"))
		{
			break;
		}
	}

	//判断使用的方法正确
	if (strcmp(method, "GET") != 0)
	{
		send_err(400, "browser error", "wrong method");
	}

	if (path[0] != '/')
	{
		send_err(400, "browser error", "dir not found");
	}

	//将/hello.c 取出 hello.c
	file = path + 1;

	//*****此处开始判断读取的是文件还是目录，可以使用stat函数读取文件属性
	//使用stat函数判断文件是否存在
	if (stat(file, &sbuf) < 0)
	{
		send_err(400, "browser error", "file exit");
	}

	//判断是目录还是文件
	if (S_ISDIR(sbuf.st_mode))
	{
		send_header(200, "OK", "text/html;charset=utf-8");
		printf("<html>"
			"<head><title>Index of %s</title></head>"
			"<body bgcolor=\"#cc99cc\">"
			"<h4>Index of %s</h4>"
			"<pre>"
			, file, file);
		struct dirent** d1;
		int nf = scandir(file, &d1, NULL, alphasort);
		if (nf < 0)
		{
			perror("scanfdir");
		}
		else
		{
			struct stat fst;
			char stfile[N];
			int i;
			for (i = 0; i < nf; i++)
			{
				strcpy(stfile, file);
				strcat(stfile, "/");
				strcat(stfile, dl[i]->d_name);
				if (lstat(stfile, &fst) < 0)
					printf("<a href=\"%s%s/\">%-32.32s/</a>", file + 1, dl[i]->d_name, dl[i]->d_name);
				else if (S_ISDIR(fst.st_mode))
					printf("<a href=\"%s%s/\">%-32.32s/</a> \t\t%14lld", file + 1, dl[i]->d_name, dl[i]->d_name, (long long)fst.st_size);
				else
					printf("<a href=\"%s%s\">%-32.32s</a> \t\t%14lld", file + 1, dl[i]->d_name, dl[i]->d_name, (long long)fst.st_size);
				printf("<br/>");
			}
		}
		printf("</pre>"
			"</body>"
			"</html>");
	}
	else    //普通文件
	{

		//打开文件,浏览器只读打开就可以了
		fp = fopen(file, "r");
		if (fp == NULL)
		{
			send_err(400, "browser error", "open file");
		}

		//打开文件之后，发送协议头，判断文件格式
		send_header(200, "send header", get_filetype(file));

		//读取文件二进制内容，输出到Web浏览器上
		while ((ich = getc(fp)) != EOF)   //用int类型的ich判断EOF,读取字符流
		{
			putchar(ich);
		}

		//刷新缓存区
		fflush(stdout);
		//关闭文件
		fclose(fp);
		fp = NULL;
	}
	return 0;
}

/////*****以下为 发送协议头，发送错误号等子函数代码******//////

//发送http协议头
void send_header(int status, char* title, char *type)
{
	if (title == NULL || type == NULL)
	{
		title = "ERROR";
		type = "text/plain; charset = iso-8859-1";
	}
	printf("HTTP/1.1 %d %s\r\n", status, title);   //HTTP协议头
	printf("Content-Type:%s\r\n", type);		   //发送数据文件类型
	printf("\r\n");
}

//发送错误号
void send_err(int status, char* title, char* describle)
{
	if (title == NULL)
	{
		title = "ERROR";
	}
	if (describle == NULL)
	{
		describle = "NONE";
	}
	send_header(status, title, "text/html; charset = iso-8859-1");    //首先给服务器发送协议头
	printf("<html>\n"													//html文件格式
		"<head><title>%d %s</title></head>\n"                   //bgcolor 背景颜色 
		"<body bgcolor=\"#cc99cc\">\n"
		"<h4>%s</h4>\n"                                     //h4字体大小   
		"<hr>\n"
		"<address>\n"
		"<a herf=\"http://www.baidu.com/\"></a>\n"          //网页链接
		"</address>\n"
		"</body>\n"
		"</html>",
		status, title, describle);
	fflush(stdout);             //刷新缓存区
	exit(1);
}

//判断文件类型
char* get_filetype(const char* file)
{
	if (file == NULL)
	{
		return NULL;
	}
	char *dot = strrchr(file, '.');     //strrchr函数可以找到‘.’在字符串中最后出现位置，并返回指向该位置起一直到字符串结束的所有字符
	if (*dot == '\0')
	{
		return "text/plain; charset = iso-8859-1";     //无格式正文
	}
	else if (strcmp(dot, ".html") == 0)
	{
		return "text/html; charset = iso-8859-1";      //html格式正文
	}
	else if (strcmp(dot, ".jpg") == 0)
	{
		return "image/jpeg";						   //jpeg格式图像
	}
	else if (strcmp(dot, ".gif") == 0)
	{
		return "image/gif";                            //gif格式图像
	}
	else if (strcmp(dot, ".png") == 0)
	{
		return "image/png";                            //png格式图像
	}
	else if (strcmp(dot, ".wav") == 0)
	{
		return "audio/wav";                            //wav格式音频
	}
	else if (strcmp(dot, ".mp3") == 0)
	{
		return "audio/mpeg";                           //wav格式音频
	}
	else if (strcmp(dot, ".avi") == 0)
	{
		return "video/x-msvideo";                      //avi格式视频
	}
	else if (strcmp(dot, ".mov") == 0)
	{
		return "video/quicktime";                      //mov格式视频
	}
	else
	{
		return "text/plain; charset = iso-8859-1";      //无格式正文
	}
}
