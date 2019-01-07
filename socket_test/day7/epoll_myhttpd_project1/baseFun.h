/*************************************************************************
    > File Name: baseFun.h
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Mon 07 Jan 2019 06:55:40 PM CST
 ************************************************************************/

#ifndef _BASEFUN_H_
#define _BASEFUN_H_

//16进制数转化为10进制, return 0不会出现 
int hexit(char c);

//中文字符解码
void decode_str(char *to, char *from);

//"编码"，用作回写浏览器的时候，将除字母数字及/_.-~以外的字符转义后回写。
//strencode(encoded_name, sizeof(encoded_name), name);
void encode_str(char* to, size_t tosize, const char* from);

//获取浏览器请求的文件的类型
const char *get_file_type(const char *name);

//获取一行/r/n 结尾的数据
int get_line(int cfd, char *buf, int size);

void sys_err(const char *arg);

#endif

