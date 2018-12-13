/*************************************************************************
    > File Name: 07_json_parse.c
    > Author: yanghang
    > Mail: 2459846416@qq.com 
    > Created Time: Tue 04 Dec 2018 08:01:12 PM CST
 ************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"cJSON.h"

int main(void)
{
	char *string = "{\"family\":[\"father\",\"mother\",\"brother\",\"sister\",\"others\"]}";

	//解析成cjson格式，存储在结构体中
	cJSON *json = cJSON_Parse(string);
	cJSON *node = NULL;

	//根据键 key 来查找 json 节点，如果找到了返回非空，如果没找到返回空
	node = cJSON_GetObjectItem(json, "family");
	if(node == NULL){
		printf("family node == NULL\n");
	}else{
		rintf("found family node\n");
	}

	node = cJSON_GetObjectItem(json, "school");
	if(node == NULL){
		printf("school node == NULL\n");
	}
	else{
		printf("found school node\n");
	}
	
	//判断获取的节点类型，如果是数组，获取数组的元素个数大小
	node = cJSON_GetObjectItem(json, "family");
	if(node->type == cJSON_Array)
	{
		printf("array size is %d\n", cJSON_GetArraySize(node));
	}

	//遍历数组，此时　node　是从对象中获得的数组
	cJSON *tnode = NULL;
	int size = cJSON_GetArraySize(node);
	int i;
	for(i = 0;i < size;++i){
		tnode = cJSON_GetArrayItem(node, i);

		if(tnode->type == cJSON_String){
			printf("value[%d]:%s\n", i, tnode->valuestring);
		}else{
			printf("node's type is not string\n");
		}
	}

	//是一个宏函数，用作循环
	cJSON_ArrayForEach(tnode, node){
		if(tnode->type == cJSON_String){
			printf("in foreach: value :%s\n", tnode->valuestring);
		}else{
			printf("node's type is not string\n");
		}
	}
	
	return 0;
}


