/*************************************************************************
    > File Name: 05_testjson.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Mon 03 Dec 2018 07:15:48 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"cJSON.h"

int main(void)
{
	//创建一个jSON文档对象 { }
	cJSON *json = cJSON_CreateObject();

	//向文档中　添加一个键值对　｛"name":"张子繁"｝
	cJSON_AddItemToObject(json,  "name", cJSON_CreateString("张子繁"));

	//向文档中　添加一个键值对　｛"age":29｝
	cJSON_AddItemToObject(json,  "age", cJSON_CreateNumber(29));

	cJSON *array = NULL;
	//向文档中　添加一个数组　｛"love":[]｝
	cJSON_AddItemToObject(json,  "love", array = cJSON_CreateArray());
	
	//向数组中添加数据
	cJSON_AddItemToArray(array, cJSON_CreateString("LOL"));
	cJSON_AddItemToArray(array, cJSON_CreateString("NBA"));
	cJSON_AddItemToArray(array, cJSON_CreateString("Shopping"));

	//添加指定类型数据到jSON文档
	cJSON_AddNumberToObject(json,  "score", 99);
	cJSON_AddStringToObject(json,  "address", "三魁");

	//将jSON结构格式化写到缓冲区中
	char *buf = cJSON_Print(json);	

	//打开文件写入
	FILE *fp = fopen("05_create.json", "a");
	
	//写入文件中
	fwrite(buf, 1, strlen(buf), fp);

	free(buf);
	fclose(fp);
	cJSON_Delete(json);

	return 0;
}
