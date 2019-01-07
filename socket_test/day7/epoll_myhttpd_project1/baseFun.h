/*************************************************************************
    > File Name: baseFun.h
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Mon 07 Jan 2019 06:55:40 PM CST
 ************************************************************************/

#ifndef _BASEFUN_H_
#define _BASEFUN_H_

//16������ת��Ϊ10����, return 0������� 
int hexit(char c);

//�����ַ�����
void decode_str(char *to, char *from);

//"����"��������д�������ʱ�򣬽�����ĸ���ּ�/_.-~������ַ�ת����д��
//strencode(encoded_name, sizeof(encoded_name), name);
void encode_str(char* to, size_t tosize, const char* from);

//��ȡ�����������ļ�������
const char *get_file_type(const char *name);

//��ȡһ��/r/n ��β������
int get_line(int cfd, char *buf, int size);

void sys_err(const char *arg);

#endif

