/*************************************************************************
    > File Name: 05_testjson.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Mon 04 Dec 2018 08:31:48 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"cJSON.h"

int main(void)
{
	//创建一个jSON文档对象 { }
	cJSON *json = cJSON_CreateObject();

	cJSON_AddItemToObject(json, "country", cJSON_CreateString("China"));

	cJSON *array = NULL;
	cJSON_AddItemToObject(json, "star", array = cJSON_CreateArray());

	cJSON *obj = NULL;
	cJSON_AddItemToArray(array, obj = cJSON_CreateObject());
	cJSON_AddItemToObject(obj, "name", cJSON_CreateString("Faye"));
	cJSON_AddStringToObject(obj, "address", "beijing");

	cJSON_AddItemToArray(array, obj = cJSON_CreateObject());
	cJSON_AddStringToObject(obj, "name", "Andy");
	cJSON_AddStringToObject(obj, "address", "HOngKong");

	cJSON_AddItemToArray(array, obj = cJSON_CreateObject());
	cJSON_AddStringToObject(obj, "name", "LiuDH");
	cJSON_AddStringToObject(obj, "address", "zhoushan");

	//将jSON结构格式化写到缓冲区中
	char *buf = cJSON_Print(json);	

	//打开文件写入
	FILE *fp = fopen("06_create.json", "a");
	
	//写入文件中
	fwrite(buf, 1, strlen(buf), fp);

	free(buf);
	fclose(fp);
	cJSON_Delete(json);

	return 0;
}
