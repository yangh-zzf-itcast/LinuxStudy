#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define N 4096

//���ļ��汾

//����httpЭ��ͷ
void send_headers(char *type)
{
	HTTP / 1.1 200 OK;
	Content - Type:Text / plain : charset = iso - 8859 - 1;
    Connection:close;
	\r\n
}

//���ʹ����
void send_err(����ţ��������ƣ���������)
{
	HTTP / 1.1 200 OK;
	Content - Type:Text / plain : charset = iso - 8859 - 1;
	\r\n
	<html>
		<head><title>����� ��������</title></head>
    <body>
	��������
	</body>
	</html>
}

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
	if(argc !=2 )     // ./xhttpd   /home/yangh/dir
		//send_err();

    //���Ĺ���Ŀ¼�� /home/yangh/xhttpdir ��
	if(chdir(argv[1])==-1)
		//send_err();

    if(fgets(line,N,stdin)==NULL)    //GET /hello.c HTTP/1.1   ----������strtok���������ո�ָ�
		//send_err();
	
	//��ֶ����ĵ�һ�У����ڶ��������ļ�·�� /hello.c ȡ��
	if(sscanf(line,"%[^ ] %[^ ] %[^ ]", method, path, protocol) != 3)    //��ֵõ��ķ�����Ҫ���ʵ��ļ����Լ�HTTPЭ��ͷ
																		 //��������ʽ�ķ���	%[^ ]
	   //send_err();
    
    //��ʣ�ಿ�ֶ���Ϊֹ��/r/n����
    while(fgets(line,N,stdin)!=NULL)
		if(strcmp(line,"\r\n"))
			break

	//�ж�ʹ�õķ�����ȷ
	if(strcmp(methd, "GET")!=0)
	   //send_err()

    //��/hello.c ȡ�� hello.c
	file=path+1
    

    //*****�˴���ʼ�ж϶�ȡ�����ļ�����Ŀ¼������ʹ��stat������ȡ�ļ�����
    //ʹ��stat�����ж��ļ��Ƿ����
    if(stat(file,&sbuf)<0)
		//send_err

	//���ļ�
	fp = fopen(file,"r")     //�����ֻ���򿪾Ϳ�����
    if(fp==NULL)
		//send_err


    //send_header

    while((ich=getc(fp))!=EOF)
		putchar(ich)
    
	//ˢ�»�����
    fflush(stdout)

	fclose(fp)

    return 0
} 