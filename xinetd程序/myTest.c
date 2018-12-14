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

//���ļ��汾

//����httpЭ��ͷ
void send_header(int status, char* title, char *type);


//���ʹ����
void send_err(int status, char* title, char* describle);

//�ж��ļ�����
char* get_filetype(const char* file);


//������
//xinetd --> ִ��./xhttpd==argv[0] ����argv[1]==/home/yangh/xhttpdir==������е� '/',����������Ŀ¼
int main(int argc, char *argv[])
{
	char line[N];
	char method[N], path[N], protocol[N];

	char *file;
	struct stat sbuf;
	FILE *fp;
	int ich;

	//����ʧ�ܣ�����
	if (argc != 2)     // ./xhttpd   /home/yangh/dir
	{
		send_err(500, "server error", "argv <2");
	}

	//���Ĺ���Ŀ¼�� /home/yangh/xhttpdir ��
	if (chdir(argv[1]) == -1)
	{
		send_err(500, "server error", "chdir error");
	}

	//GET /hello.c HTTP/1.1   ----������strtok���������ո�ָ�
	if (fgets(line, N, stdin) == NULL)
	{
		send_err(500, "server error", "type path protocol");
	}

	//��ֶ����ĵ�һ�У����ڶ��������ļ�·�� /hello.c ȡ��
	if (sscanf(line, "%[^ ] %[^ ] %[^ ]", method, path, protocol) != 3)    //��ֵõ��ķ�����Ҫ���ʵ��ļ����Լ�HTTPЭ��ͷ
	{																	   //��������ʽ�ķ���	%[^ ]
		send_err(400, "browser error", "bad request");
	}

	//��ʣ�ಿ�ֶ���Ϊֹ��/r/n����
	while (fgets(line, N, stdin) != NULL)
	{
		if (strcmp(line, "\r\n"))
		{
			break;
		}
	}

	//�ж�ʹ�õķ�����ȷ
	if (strcmp(method, "GET") != 0)
	{
		send_err(400, "browser error", "wrong method");
	}

	if (path[0] != '/')
	{
		send_err(400, "browser error", "dir not found");
	}

	//��/hello.c ȡ�� hello.c
	file = path + 1;

	//*****�˴���ʼ�ж϶�ȡ�����ļ�����Ŀ¼������ʹ��stat������ȡ�ļ�����
	//ʹ��stat�����ж��ļ��Ƿ����
	if (stat(file, &sbuf) < 0)
	{
		send_err(400, "browser error", "file exit");
	}

	//�ж���Ŀ¼�����ļ�
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
	else    //��ͨ�ļ�
	{

		//���ļ�,�����ֻ���򿪾Ϳ�����
		fp = fopen(file, "r");
		if (fp == NULL)
		{
			send_err(400, "browser error", "open file");
		}

		//���ļ�֮�󣬷���Э��ͷ���ж��ļ���ʽ
		send_header(200, "send header", get_filetype(file));

		//��ȡ�ļ����������ݣ������Web�������
		while ((ich = getc(fp)) != EOF)   //��int���͵�ich�ж�EOF,��ȡ�ַ���
		{
			putchar(ich);
		}

		//ˢ�»�����
		fflush(stdout);
		//�ر��ļ�
		fclose(fp);
		fp = NULL;
	}
	return 0;
}

/////*****����Ϊ ����Э��ͷ�����ʹ���ŵ��Ӻ�������******//////

//����httpЭ��ͷ
void send_header(int status, char* title, char *type)
{
	if (title == NULL || type == NULL)
	{
		title = "ERROR";
		type = "text/plain; charset = iso-8859-1";
	}
	printf("HTTP/1.1 %d %s\r\n", status, title);   //HTTPЭ��ͷ
	printf("Content-Type:%s\r\n", type);		   //���������ļ�����
	printf("\r\n");
}

//���ʹ����
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
	send_header(status, title, "text/html; charset = iso-8859-1");    //���ȸ�����������Э��ͷ
	printf("<html>\n"													//html�ļ���ʽ
		"<head><title>%d %s</title></head>\n"                   //bgcolor ������ɫ 
		"<body bgcolor=\"#cc99cc\">\n"
		"<h4>%s</h4>\n"                                     //h4�����С   
		"<hr>\n"
		"<address>\n"
		"<a herf=\"http://www.baidu.com/\"></a>\n"          //��ҳ����
		"</address>\n"
		"</body>\n"
		"</html>",
		status, title, describle);
	fflush(stdout);             //ˢ�»�����
	exit(1);
}

//�ж��ļ�����
char* get_filetype(const char* file)
{
	if (file == NULL)
	{
		return NULL;
	}
	char *dot = strrchr(file, '.');     //strrchr���������ҵ���.�����ַ�����������λ�ã�������ָ���λ����һֱ���ַ��������������ַ�
	if (*dot == '\0')
	{
		return "text/plain; charset = iso-8859-1";     //�޸�ʽ����
	}
	else if (strcmp(dot, ".html") == 0)
	{
		return "text/html; charset = iso-8859-1";      //html��ʽ����
	}
	else if (strcmp(dot, ".jpg") == 0)
	{
		return "image/jpeg";						   //jpeg��ʽͼ��
	}
	else if (strcmp(dot, ".gif") == 0)
	{
		return "image/gif";                            //gif��ʽͼ��
	}
	else if (strcmp(dot, ".png") == 0)
	{
		return "image/png";                            //png��ʽͼ��
	}
	else if (strcmp(dot, ".wav") == 0)
	{
		return "audio/wav";                            //wav��ʽ��Ƶ
	}
	else if (strcmp(dot, ".mp3") == 0)
	{
		return "audio/mpeg";                           //wav��ʽ��Ƶ
	}
	else if (strcmp(dot, ".avi") == 0)
	{
		return "video/x-msvideo";                      //avi��ʽ��Ƶ
	}
	else if (strcmp(dot, ".mov") == 0)
	{
		return "video/quicktime";                      //mov��ʽ��Ƶ
	}
	else
	{
		return "text/plain; charset = iso-8859-1";      //�޸�ʽ����
	}
}
