/*************************************************************************
    > File Name: 03_testjson.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Sun 02 Dec 2018 09:20:39 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>

#include"cJSON.h"

int main(void)
{
	//　\" = "  表示转义字符
	char *data = "{\"love\":[\"LOL\", \"Go shopping\"]}";

	//解析 data 字符串，存储成cJSON　结构体
	cJSON *json = cJSON_Parse(data);

	char *json_data = NULL;

	printf("data:%s\n", json_data = cJSON_Print(json));

	free(json_data);

	cJSON_Delete(json);

	return 0;
}

